#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <cstdlib>
#include <atomic>
#include <cstdarg>
#include "SimConnect.h"
#include "SimTelemetry.h"
#include "RawTelemetry.h"
#include "SerialLink.h"
#include "PicoCsv.h"

FILE* logFile=nullptr;
void Log(const char* format, ...) {
    va_list args; va_start(args,format); std::vprintf(format,args); va_end(args); std::fflush(stdout);
    if(logFile) { va_start(args,format); std::vfprintf(logFile,format,args); va_end(args); std::fflush(logFile); }
}
struct Telemetry { double value[YokeRaw::Count]; };
HANDLE sim=nullptr, port=INVALID_HANDLE_VALUE;
std::atomic<bool> quit{false};
bool failed=false, paused=true, running=false, opened=false, enable=false, gusts=false, buffets=true;
bool pauseKnown=false; // Startup default is not confirmation of simulator pause.
unsigned samples=0;
uint16_t sequence=0;
ULONGLONG lastData=0, lastSend=0;
double referenceKnots=120, gustGain=1, gustLongitudinal=0.25;
FILE* csv=nullptr;
FILE* picoLog=nullptr;
FILE* picoCsv=nullptr;
FILE* gainsCsv=nullptr;
unsigned picoLines=0, badPicoLines=0;
ULONGLONG lastCsvFlush=0;
YokeCsv::LineSplitter picoSplitter;
YokeRaw::Sample latest;
bool pending=false;
YokeLink::LinkMonitor link;
YokeLink::Reconnector reconnect;
YokeLink::BootLineScanner bootScanner;
std::string com;
void Stamp(char out[16]) {
    SYSTEMTIME t; GetLocalTime(&t);
    sprintf_s(out,16,"%02u:%02u:%02u",unsigned(t.wHour),unsigned(t.wMinute),unsigned(t.wSecond));
}
void ClosePort() {
    if(port!=INVALID_HANDLE_VALUE) { CloseHandle(port); port=INVALID_HANDLE_VALUE; }
}
void LinkLost(const char* motivo, unsigned long codigo, unsigned long escritos) {
    DWORD errores=0; COMSTAT estado{};
    if(port!=INVALID_HANDLE_VALUE) ClearCommError(port,&errores,&estado);
    char t[16]; Stamp(t);
    Log("[%s] ERROR enlace USB con la Pico: %s (codigo Windows %lu, bytes %lu/%u, cola salida %lu, errores 0x%lX). Sin tramas validas el firmware desarma por watchdog. ",
        t,motivo,codigo,escritos,unsigned(YokeRaw::packetSize),estado.cbOutQue,errores);
    if(!quit) Log("Reconectando %s...\n",com.c_str());
    else Log("Fin del puente.\n");
    ClosePort(); reconnect.onLost(GetTickCount64());
}
BOOL WINAPI Stop(DWORD) { quit=true; return TRUE; }
bool Check(HRESULT hr, const char* op) {
    if(SUCCEEDED(hr)) return true;
    Log("ERROR %s 0x%08lX\n",op,(unsigned long)hr); failed=quit=true; return false;
}
// Raw input records and selected outgoing packets; no estimated forces on PC.
void Record(bool sent,bool armed) {
    if(!csv) return;
    std::fprintf(csv,"%llu,%lu,%u",GetTickCount64(),(unsigned long)latest.timeMs,samples);
    for(int i=0;i<YokeRaw::Count;++i) std::fprintf(csv,",%.9g",double(latest.value[i]));
    std::fprintf(csv,",%d,%d,%d,%d,%.3f,%.3f,%.3f,%d,%d\n",paused,running,armed,sent,
        referenceKnots,gustGain,gustLongitudinal,gusts,buffets);
    // Flush on each transmitted snapshot, not on every queued simulator callback.
    if(sent) std::fflush(csv);
}
void Send(bool armed) {
    if(port==INVALID_HANDLE_VALUE || !reconnect.armAllowed(GetTickCount64())) armed=false;
    const bool confirmedPause=opened && pauseKnown && paused && !failed && !quit;
    YokeRaw::Sample payload=latest;
    payload.referenceKnots=float(referenceKnots); payload.gustGain=float(gustGain);
    payload.gustLongitudinal=float(gustLongitudinal);
    payload.options=uint8_t((gusts?YokeRaw::Gusts:0)|(buffets?YokeRaw::Buffets:0));
    // Transport validation only. Invalid mandatory input must still send a valid disarm.
    if(!YokeRaw::valid(payload)) { payload=YokeRaw::Sample{}; armed=false; }
    if(confirmedPause) armed=false;
    uint8_t frame[YokeRaw::packetSize]={0xa5,0x5a,8,uint8_t(confirmedPause ? 2 : (armed?1:0))};
    YokeSC::put16(frame+4,++sequence);
    YokeRaw::encodePayload(frame,payload);
    YokeSC::put16(frame+111,YokeSC::crc(frame,111));
    bool wrote=false;
    if(port!=INVALID_HANDLE_VALUE) {
        DWORD written=0;
        const BOOL ok=WriteFile(port,frame,sizeof(frame),&written,nullptr);
        const DWORD error=ok?0:GetLastError();
        const auto outcome=YokeLink::classifyWrite(ok!=FALSE,written,sizeof(frame),error);
        wrote=outcome==YokeLink::WriteOutcome::Ok;
        if(link.onWrite(outcome,GetTickCount64()))
            LinkLost(outcome==YokeLink::WriteOutcome::Timeout?"escritura vencida":"error de dispositivo",error,written);
    }
    lastSend=GetTickCount64();
    Record(wrote,wrote&&armed);
}
void CALLBACK Receive(SIMCONNECT_RECV* data,DWORD bytes,void*) {
    switch(data->dwID) {
    case SIMCONNECT_RECV_ID_OPEN: {
        opened=true;
        auto* o=reinterpret_cast<SIMCONNECT_RECV_OPEN*>(data);
        Log("Conectado: %s %lu.%lu\n",o->szApplicationName,o->dwApplicationVersionMajor,o->dwApplicationVersionMinor);
        const char* names[]={"DYNAMIC PRESSURE","AIRSPEED INDICATED","AIRCRAFT WIND X","AIRCRAFT WIND Y","AIRCRAFT WIND Z","SIM ON GROUND","IS SLEW ACTIVE",
            "INCIDENCE ALPHA","STALL ALPHA","TRAILING EDGE FLAPS LEFT PERCENT",
            "TRAILING EDGE FLAPS RIGHT PERCENT","DESIGN SPEED VS0","DESIGN SPEED VS1",
            "GEAR LEFT POSITION","IS GEAR RETRACTABLE","SPOILERS LEFT POSITION","SPOILERS RIGHT POSITION","GROUND VELOCITY","YOKE X POSITION","YOKE Y POSITION","ACCELERATION BODY X","ACCELERATION BODY Y"};
        const char* units[]={"psf","knots","knots","knots","knots","Bool","Bool",
            "degrees","degrees","percent over 100","percent over 100","knots","knots",
            "percent over 100","Bool","percent over 100","percent over 100","knots","Position","Position","feet per second squared","feet per second squared"};
        for(int i=0;i<22;++i) if(!Check(SimConnect_AddToDataDefinition(sim,1,names[i],units[i],SIMCONNECT_DATATYPE_FLOAT64),names[i])) return;
        Check(SimConnect_SubscribeToSystemEvent(sim,2,"Pause"),"Pause");
        Check(SimConnect_SubscribeToSystemEvent(sim,3,"Sim"),"Sim");
        Check(SimConnect_RequestDataOnSimObject(sim,1,1,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME),"telemetry");
        break;
    }
    case SIMCONNECT_RECV_ID_EVENT: {
        auto* e=reinterpret_cast<SIMCONNECT_RECV_EVENT*>(data);
        if(e->uEventID==2) { paused=e->dwData!=0; pauseKnown=true; }
        if(e->uEventID==3) running=e->dwData!=0;
        if(e->uEventID==2 || paused || !running) { Send(false); pending=false; }
        break;
    }
    case SIMCONNECT_RECV_ID_SYSTEM_STATE: {
        auto* e=reinterpret_cast<SIMCONNECT_RECV_SYSTEM_STATE*>(data);
        if(e->dwRequestID==2) { paused=e->dwInteger!=0; pauseKnown=true; }
        if(e->dwRequestID==3) running=e->dwInteger!=0;
        break;
    }
    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
        auto* d=reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA*>(data);
        const size_t offset=reinterpret_cast<char*>(&d->dwData)-reinterpret_cast<char*>(data);
        if(d->dwRequestID!=1||d->dwDefineCount!=22||bytes<offset+sizeof(Telemetry)) return;
        Telemetry t{}; std::memcpy(&t,&d->dwData,sizeof(t));
        const ULONGLONG now=GetTickCount64();
        for(int i=0;i<YokeRaw::Count;++i) latest.value[i]=float(t.value[i]);
        latest.timeMs=uint32_t(now);
        lastData=now; pending=true; ++samples;
        Record(false,false); // Received only: the packet is selected after dispatch drains.
        if(samples%100==1) Log("SC8 muestras=%u suelo=%.0f velocidad=%.1f kt ax=%.3f ay=%.3f; calculo en Pico\n",
            samples,latest.value[YokeRaw::OnGround],latest.value[YokeRaw::GroundSpeed],
            latest.value[YokeRaw::AccelX],latest.value[YokeRaw::AccelY]);
        break;
    }
    case SIMCONNECT_RECV_ID_EXCEPTION: {
        auto* e=reinterpret_cast<SIMCONNECT_RECV_EXCEPTION*>(data);
        Log("EXCEPTION=%lu SendID=%lu Index=%lu\n",e->dwException,e->dwSendID,e->dwIndex);
        failed=quit=true; Send(false); break;
    }
    case SIMCONNECT_RECV_ID_QUIT: quit=true; Send(false); break;
    }
}
bool OpenPort(const std::string& name, unsigned handshakeMs) {
    if(name.size()<4||name.substr(0,3)!="COM"||name.find_first_not_of("0123456789",3)!=std::string::npos) return false;
    port=CreateFileA(("\\\\.\\"+name).c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_EXISTING,0,nullptr);
    if(port==INVALID_HANDLE_VALUE) return false;
    DCB dcb{}; dcb.DCBlength=sizeof(dcb);
    if(!GetCommState(port,&dcb)) { ClosePort(); return false; }
    dcb.BaudRate=115200; dcb.ByteSize=8; dcb.Parity=NOPARITY; dcb.StopBits=ONESTOPBIT;
    dcb.fBinary=TRUE; dcb.fParity=FALSE; dcb.fDtrControl=DTR_CONTROL_ENABLE;
    dcb.fOutxCtsFlow=FALSE; dcb.fOutxDsrFlow=FALSE; dcb.fOutX=FALSE; dcb.fInX=FALSE;
    if(!SetCommState(port,&dcb)) { ClosePort(); return false; }
    COMMTIMEOUTS timeouts{}; timeouts.ReadIntervalTimeout=MAXDWORD; timeouts.WriteTotalTimeoutConstant=10;
    if(!SetCommTimeouts(port,&timeouts)) { ClosePort(); return false; }
    PurgeComm(port,PURGE_RXCLEAR|PURGE_TXCLEAR);
    std::string response;
    const ULONGLONG start=GetTickCount64(); ULONGLONG query=0;
    while(!quit && GetTickCount64()-start<handshakeMs) {
        if(GetTickCount64()-query>200) { DWORD n=0; if(!WriteFile(port,"?",1,&n,nullptr)) { ClosePort(); return false; } query=GetTickCount64(); }
        char b[256]; DWORD n=0; if(!ReadFile(port,b,sizeof(b),&n,nullptr)) { ClosePort(); return false; }
        response.append(b,n);
        if(response.find("YOKE17SC8\n")!=std::string::npos) return true;
        if(response.size()>1024) response.erase(0,response.size()-128);
        Sleep(10);
    }
    ClosePort(); return false;
}
void PollPort(ULONGLONG now) {
    if(port!=INVALID_HANDLE_VALUE) {
        char b[4096]; DWORD n=0;
        if(!ReadFile(port,b,sizeof(b),&n,nullptr)) LinkLost("error de lectura",GetLastError(),0);
        else if(n) {
            if(picoLog) { std::fwrite(b,1,n,picoLog); std::fflush(picoLog); }
            bootScanner.feed(b,n,[](const char* l){ char t[16]; Stamp(t); Log("[%s] Pico: %s\n",t,l); });
            picoSplitter.feed(b,n,[](const char* l) {
                if((l[0]!='M' && l[0]!='G') || l[1]!=',') return;
                ++picoLines; char row[512];
                const bool model=l[0]=='M';
                if(!(model ? YokeCsv::modelRow(l,row,sizeof(row)) : YokeCsv::gainsRow(l,row,sizeof(row)))) {
                    ++badPicoLines; return;
                }
                FILE* target=model ? picoCsv : gainsCsv;
                if(target) std::fprintf(target,"%llu,%s\n",GetTickCount64(),row);
            });
        }
    }
    if(!com.empty() && port==INVALID_HANDLE_VALUE && reconnect.shouldAttempt(now)) {
        if(reconnect.shouldLogRetry(now)) { char t[16]; Stamp(t); Log("[%s] Reintentando %s...\n",t,com.c_str()); }
        if(OpenPort(com,YokeLink::kReconnectHandshakeMs)) {
            const ULONGLONG t=GetTickCount64(); char s[16]; Stamp(s);
            Log("[%s] Enlace USB recuperado en %s tras %.1f s; armado diferido %llu ms.\n",
                s,com.c_str(),(t-reconnect.lostAtMs)/1000.0,(unsigned long long)YokeLink::kRearmHoldoffMs);
            reconnect.onReopened(t); link.reset(); bootScanner=YokeLink::BootLineScanner{}; picoSplitter=YokeCsv::LineSplitter{};
        } else reconnect.onAttemptFailed(GetTickCount64());
    }
    if(now-lastCsvFlush>=200) {
        if(picoCsv) std::fflush(picoCsv);
        if(gainsCsv) std::fflush(gainsCsv);
        lastCsvFlush=now;
    }
}
int main(int argc,char** argv) {
    double duration=20; bool durationSpecified=false; // Double click: 20-second monitor, no motor commands.
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--enable") enable=true;
        else if(arg=="--gusts") gusts=true;
        else if(arg=="--no-buffets") buffets=false;
        else if(arg=="--gust-gain"&&i+1<argc) {
            const char* start=argv[++i]; char* end=nullptr;
            gustGain=std::strtod(start,&end);
            if(end==start||*end||!std::isfinite(gustGain)||gustGain<0||gustGain>10) {
                Log("--gust-gain debe ser un numero entre 0 y 10.\n"); return 2;
            }
        }
        else if(arg=="--gust-longitudinal"&&i+1<argc) {
            const char* start=argv[++i]; char* end=nullptr;
            gustLongitudinal=std::strtod(start,&end);
            if(end==start||*end||!std::isfinite(gustLongitudinal)||std::fabs(gustLongitudinal)>1) {
                Log("--gust-longitudinal debe estar entre -1 y 1.\n"); return 2;
            }
        }
        else if(arg=="--port"&&i+1<argc) com=argv[++i];
        else if(arg=="--seconds"&&i+1<argc) { duration=std::atof(argv[++i]); durationSpecified=true; }
        else if(arg=="--reference-knots"&&i+1<argc) referenceKnots=std::atof(argv[++i]);
        else { Log("Uso: Bridge.exe [--seconds 20] [--port COMn --enable] [--gusts --gust-gain 0..10 --gust-longitudinal -1..1] [--reference-knots 120] [--no-buffets]\n"); return 2; }
    }
    if(!com.empty()&&!durationSpecified) duration=0;
    if(!std::isfinite(referenceKnots)||referenceKnots<40||referenceKnots>250||!std::isfinite(duration)||duration<0|| (enable&&com.empty())) return 2;
    char file[MAX_PATH]; GetModuleFileNameA(nullptr,file,MAX_PATH); char* tail=std::strrchr(file,'\\');
    if(tail) {
        strcpy_s(tail+1,MAX_PATH-(tail+1-file),"pico.log");
        WIN32_FILE_ATTRIBUTE_DATA attributes{};
        if(GetFileAttributesExA(file,GetFileExInfoStandard,&attributes) &&
           (attributes.nFileSizeHigh || attributes.nFileSizeLow)) {
            const char* current[]={"bridge.log","telemetry.csv","pico.log","pico.csv","ganancias.csv"};
            const char* previous[]={"bridge.anterior.log","telemetry.anterior.csv","pico.anterior.log","pico.anterior.csv","ganancias.anterior.csv"};
            for(unsigned i=0;i<5;++i) {
                strcpy_s(tail+1,MAX_PATH-(tail+1-file),current[i]);
                char destination[MAX_PATH]; strcpy_s(destination,file);
                strcpy_s(destination+(tail+1-file),MAX_PATH-(tail+1-file),previous[i]);
                MoveFileExA(file,destination,MOVEFILE_REPLACE_EXISTING);
            }
        }
    }
    if(tail) { strcpy_s(tail+1,MAX_PATH-(tail+1-file),"bridge.log"); fopen_s(&logFile,file,"w"); }
    if(tail) { strcpy_s(tail+1,MAX_PATH-(tail+1-file),"telemetry.csv"); fopen_s(&csv,file,"w"); }
    if(tail) { strcpy_s(tail+1,MAX_PATH-(tail+1-file),"pico.log"); fopen_s(&picoLog,file,"w"); }
    if(tail) { strcpy_s(tail+1,MAX_PATH-(tail+1-file),"pico.csv"); fopen_s(&picoCsv,file,"w"); }
    if(tail) { strcpy_s(tail+1,MAX_PATH-(tail+1-file),"ganancias.csv"); fopen_s(&gainsCsv,file,"w"); }
    if(picoCsv) std::fprintf(picoCsv,"%s\n",YokeCsv::modelHeader);
    if(gainsCsv) std::fprintf(gainsCsv,"%s\n",YokeCsv::gainsHeader);
    if(csv) std::fprintf(csv,"host_time_ms,sample_time_ms,sample_index,pressure_psf,ias_knots,wind_x_knots,wind_y_knots,wind_z_knots,on_ground,slew,aoa_deg,stall_alpha_deg,flap_left,flap_right,vs0_knots,vs1_knots,gear_fraction,gear_retractable,spoiler_left,spoiler_right,ground_speed_knots,yoke_roll,yoke_pitch,ground_accel_x_ft_s2,ground_accel_y_ft_s2,paused,running,armed,sent,reference_knots,gust_gain,gust_longitudinal,gusts_enabled,buffets_enabled\n");
    Log("Yoke SimConnect SC8: transporte de telemetria; todos los modelos en Pico 2 | %s\n",enable?"HABILITADO":"SOLO MONITOR");
    Log("Ajustes enviados: referencia %.1f kt, gustGain %.2f, longitudinal %.2f, gusts=%d buffets=%d\n",
        referenceKnots,gustGain,gustLongitudinal,gusts,buffets);
    if(!com.empty()&&!OpenPort(com,3000)) { Log("No se identifico firmware YOKE17SC8 en %s. No se enviaron fuerzas.\n",com.c_str()); ClosePort(); if(csv)std::fclose(csv); if(logFile)std::fclose(logFile); if(picoLog)std::fclose(picoLog); if(picoCsv)std::fclose(picoCsv); if(gainsCsv)std::fclose(gainsCsv); return 3; }
    SetConsoleCtrlHandler(Stop,TRUE);
    const ULONGLONG start=GetTickCount64();
    if(Check(SimConnect_Open(&sim,"Yoke SC8 raw telemetry",nullptr,0,nullptr,SIMCONNECT_OPEN_CONFIGINDEX_LOCAL),"Open")) {
        while(!quit) {
            if(!Check(SimConnect_CallDispatch(sim,Receive,nullptr),"Dispatch")) break;
            const ULONGLONG now=GetTickCount64();
            // Coalesce a callback batch: send the LATEST raw sample, not the first.
            // The timestamp describes acquisition, never refreshed for held/stale samples.
            if(pending && now-lastSend>=20) {
                const bool active=enable && opened && !failed && !quit && pauseKnown && running && !paused &&
                    now-lastData<250 && latest.value[YokeRaw::Slew]==0;
                Send(active); pending=false;
            }
            if((!lastData||now-lastData>250||paused||!running)&&now-lastSend>100) { Send(false); pending=false; }
            if(!opened&&now-start>15000) { failed=true; break; }
            if(duration>0&&now-start>=duration*1000) break;
            PollPort(now);
            Sleep(5);
        }
        opened=false; Send(false); SimConnect_Close(sim);
    }
    ClosePort();
    if(csv) std::fclose(csv);
    if(picoLog) std::fclose(picoLog);
    if(picoCsv) std::fclose(picoCsv);
    if(gainsCsv) std::fclose(gainsCsv);
    Log("Fin: muestras=%u error=%d reconexiones=%u lineas_pico=%u lineas_pico_malas=%u\n",samples,failed,reconnect.reconnections,picoLines,badPicoLines);
    return reconnect.lost ? 4 : (failed||samples==0 ? 1 : 0);
}





