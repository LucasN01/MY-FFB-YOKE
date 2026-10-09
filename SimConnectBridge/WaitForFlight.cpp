#include <Windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include "SimConnect.h"

// Read-only readiness probe: never opens a serial port or sends simulator events.
static HANDLE sim = nullptr;
static bool running = false, paused = true, pauseKnown = false, disconnected = false;
static ULONGLONG stableSince = 0, lastSample = 0;
static bool viewer = false;
static char aircraft[256] = {};
struct State { double ground, slew; char title[256]; };
static void CALLBACK Receive(SIMCONNECT_RECV* data, DWORD bytes, void*) {
    switch (data->dwID) {
    case SIMCONNECT_RECV_ID_EVENT: {
        auto* event = reinterpret_cast<SIMCONNECT_RECV_EVENT*>(data);
        if (event->uEventID == 2) { paused = event->dwData != 0; pauseKnown = true; }
        if (event->uEventID == 3) running = event->dwData != 0;
        if (!running || paused) stableSince = 0;
        break;
    }
    case SIMCONNECT_RECV_ID_SYSTEM_STATE: {
        auto* state = reinterpret_cast<SIMCONNECT_RECV_SYSTEM_STATE*>(data);
        if (state->dwRequestID == 3) running = state->dwInteger != 0;
        if (!running) stableSince = 0;
        break;
    }
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
        auto* packet = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA*>(data);
        if (packet->dwRequestID != 1 || packet->dwDefineCount != 3 ||
            bytes < offsetof(SIMCONNECT_RECV_SIMOBJECT_DATA, dwData) + sizeof(State)) break;
        State state{};
        std::memcpy(&state, &packet->dwData, sizeof(state));
        state.title[255] = 0;
        if (std::strcmp(aircraft, state.title) != 0) {
            std::memcpy(aircraft, state.title, sizeof(aircraft));
            std::printf("Vehiculo: %s\n", aircraft);
        }
        viewer = _stricmp(state.title, "Viewer") == 0;
        const auto now = GetTickCount64();
        if (!viewer && state.title[0] && running && pauseKnown && !paused && state.ground == 1 && state.slew == 0) {
            if (!stableSince || now - lastSample > 1000) stableSince = now;
        } else stableSince = 0;
        lastSample = now;
        break;
    }
    case SIMCONNECT_RECV_ID_EXCEPTION: {
        auto* error = reinterpret_cast<SIMCONNECT_RECV_EXCEPTION*>(data);
        std::printf("Error SimConnect: %lu (solicitud %lu, indice %lu). Reintentando conexion.\n",
                    error->dwException, error->dwSendID, error->dwIndex);
        disconnected = true;
        break;
    }
    case SIMCONNECT_RECV_ID_QUIT: disconnected = true; break;
    }
}
int main(int argc, char** argv) {
    // Arguments: timeout in seconds, optional Prepar3D process ID.
    const int timeout = argc > 1 ? std::atoi(argv[1]) : 1800;
    if (timeout <= 0) return 2;
    HANDLE process = argc > 2 ? OpenProcess(SYNCHRONIZE, FALSE, std::strtoul(argv[2], nullptr, 10)) : nullptr;
    if (argc > 2 && !process) return 4;
    const auto started = GetTickCount64();
    ULONGLONG nextConnect = 0, nextStatus = 0;
    int result = 1;
    while (GetTickCount64() - started < ULONGLONG(timeout) * 1000) {
        const auto now = GetTickCount64();
        if (process && WaitForSingleObject(process, 0) == WAIT_OBJECT_0) { result = 4; break; }
        if (!sim && now >= nextConnect) {
            nextConnect = now + 2000;
            if (SUCCEEDED(SimConnect_Open(&sim, "Yoke launcher readiness", nullptr, 0, nullptr, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL))) {
                running = false; paused = true; pauseKnown = false; disconnected = false; viewer = false;
                stableSince = lastSample = 0;
                if (FAILED(SimConnect_AddToDataDefinition(sim, 1, "SIM ON GROUND", "Bool", SIMCONNECT_DATATYPE_FLOAT64)) ||
                    FAILED(SimConnect_AddToDataDefinition(sim, 1, "IS SLEW ACTIVE", "Bool", SIMCONNECT_DATATYPE_FLOAT64)) ||
                    FAILED(SimConnect_AddToDataDefinition(sim, 1, "TITLE", nullptr, SIMCONNECT_DATATYPE_STRING256)) ||
                    FAILED(SimConnect_SubscribeToSystemEvent(sim, 2, "Pause")) ||
                    FAILED(SimConnect_SubscribeToSystemEvent(sim, 3, "Sim")) ||
                    FAILED(SimConnect_RequestSystemState(sim, 3, "Sim")) ||
                    FAILED(SimConnect_RequestDataOnSimObject(sim, 1, 1, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME))) {
                    result = 3; break;
                }
            } else sim = nullptr;
        }
        if (sim) {
            if (FAILED(SimConnect_CallDispatch(sim, Receive, nullptr)) || disconnected) {
                SimConnect_Close(sim); sim = nullptr; stableSince = 0;
                nextConnect = now + 5000;
            } else if (viewer && running) {
                std::puts("ERROR: Prepar3D cargo Viewer en lugar de un avion. Revisa la instalacion del vehiculo.");
                result = 5; break;
            } else if (stableSince && now >= stableSince + 2000 && now - lastSample < 1000) {
                std::puts("LISTO: avion en tierra, simulacion activa y sin pausa."); result = 0; break;
            }
        }
        if (now >= nextStatus) {
            std::printf("Esperando vuelo en tierra y sin pausa... SimConnect=%s running=%d pause=%d\n", sim ? "OK" : "esperando", running, paused);
            std::fflush(stdout); nextStatus = now + 15000;
        }
        Sleep(20);
    }
    if (sim) SimConnect_Close(sim);
    if (process) CloseHandle(process);
    return result;
}
