"""Lanza una tarea de Codex como chat visible en la app de Codex (proyecto MY FFB YOKE).

`codex exec` guarda sus sesiones con origen "exec" y la app no las muestra. Este
script inicia `codex app-server` (el protocolo que usa la app) y crea el chat con
el mismo cwd que el proyecto, asi aparece en la barra lateral como un chat normal.
Equivale a: codex exec --approve-for-me -c windows.sandbox="unelevated" --add-dir ...

Uso:
  python lanzar-codex.py "<mensaje>" [-o tmp/codex_ultimo.txt] [--add-dir DIR ...]
Codigo de salida: 0 si el turno termino "completed", 1 en otro caso.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
import threading

REPO = os.path.dirname(os.path.abspath(__file__))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mensaje")
    ap.add_argument("-o", "--salida", default=os.path.join(REPO, "tmp", "codex_ultimo.txt"))
    ap.add_argument("--add-dir", action="append", default=[])
    ap.add_argument("--titulo", default=None, help="Nombre del chat (por defecto, el mensaje)")
    args = ap.parse_args()

    codex = shutil.which("codex") or "codex"
    proc = subprocess.Popen(
        [codex, "app-server", "-c", 'windows.sandbox="unelevated"'],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
        text=True, encoding="utf-8", bufsize=1,
    )
    lock = threading.Lock()
    pendientes = {}
    siguiente = [0]

    def enviar(obj):
        with lock:
            proc.stdin.write(json.dumps(obj) + "\n")
            proc.stdin.flush()

    def pedir(metodo, params):
        siguiente[0] += 1
        rid = siguiente[0]
        ev = threading.Event()
        pendientes[rid] = [ev, None]
        enviar({"id": rid, "method": metodo, "params": params})
        ev.wait()
        msg = pendientes.pop(rid)[1]
        if "error" in msg:
            raise RuntimeError(f"{metodo}: {msg['error']}")
        return msg["result"]

    estado = {"hilo": None, "ultimo": "", "fin": threading.Event(), "status": None}

    def lector():
        for linea in proc.stdout:
            try:
                msg = json.loads(linea)
            except ValueError:
                continue
            if "id" in msg and "method" not in msg:
                p = pendientes.get(msg["id"])
                if p:
                    p[1] = msg
                    p[0].set()
            elif "id" in msg:
                # Pedido del servidor (aprobaciones): con auto_review no deberian llegar.
                # Se rechaza para no aprobar nada sin revision.
                print(f"[lanzar-codex] pedido del servidor rechazado: {msg['method']}", file=sys.stderr)
                enviar({"id": msg["id"], "result": {"decision": "decline"}})
            else:
                metodo, p = msg.get("method"), msg.get("params", {})
                if p.get("threadId") != estado["hilo"]:
                    continue
                if metodo == "item/completed" and p["item"].get("type") == "agentMessage":
                    estado["ultimo"] = p["item"].get("text", "")
                elif metodo == "turn/completed":
                    estado["status"] = p["turn"].get("status")
                    estado["fin"].set()
        estado["fin"].set()

    threading.Thread(target=lector, daemon=True).start()

    pedir("initialize", {"clientInfo": {"name": "claude_flujo", "title": "Claude (Flujo)", "version": "1.0"}})
    enviar({"method": "initialized"})

    raices = [REPO] + [os.path.abspath(d) for d in args.add_dir]
    hilo = pedir("thread/start", {
        "cwd": REPO,
        "sandbox": "workspace-write",
        "approvalPolicy": "on-request",
        "approvalsReviewer": "auto_review",
        "config": {"sandbox_workspace_write": {"writable_roots": raices}},
    })["thread"]["id"]
    estado["hilo"] = hilo
    titulo = (args.titulo or args.mensaje)[:80]
    try:
        pedir("thread/name/set", {"threadId": hilo, "name": titulo})
    except RuntimeError:
        pass
    print(f"[lanzar-codex] chat {hilo}: {titulo}", flush=True)

    pedir("turn/start", {"threadId": hilo, "input": [{"type": "text", "text": args.mensaje}]})
    estado["fin"].wait()

    os.makedirs(os.path.dirname(os.path.abspath(args.salida)), exist_ok=True)
    with open(args.salida, "w", encoding="utf-8") as f:
        f.write(estado["ultimo"])
    print(f"[lanzar-codex] turno {estado['status']}; ultimo mensaje en {args.salida}", flush=True)
    proc.stdin.close()
    try:
        proc.wait(timeout=15)
    except subprocess.TimeoutExpired:
        proc.kill()
    return 0 if estado["status"] == "completed" else 1


if __name__ == "__main__":
    sys.exit(main())
