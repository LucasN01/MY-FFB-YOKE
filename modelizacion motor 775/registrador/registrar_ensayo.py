"""Registrador de ensayos del motor 775: lee a la vez el Pro Micro (driver +
ACS712, ensayo1_rotor_bloqueado.ino) y el Arduino UNO (celda HX711,
celda_hx711.ino) y pone a cada linea la HORA DE LA PC, la misma para los dos.
Asi se puede saber que fuerza habia en la celda en cada instante del ensayo.

Por que en la PC y no en los Arduino: ninguno de los dos sabe la hora de la
PC, y sus relojes derivan (el resonador del UNO puede correrse varios
segundos por hora). La PC marca cada linea al recibirla; el retardo del USB
es de pocos ms, chico frente a las 10 muestras por segundo de la celda.

Uso (desde esta carpeta, con las dos placas conectadas):
    python registrar_ensayo.py                 (busca los puertos solo)
    python registrar_ensayo.py --micro COM4 --uno COM12
    python registrar_ensayo.py --nombre roll_pos1
Requiere pyserial (pip install pyserial).

OJO: al abrir el puerto el UNO se reinicia (con el CH340 en Windows pasa
aunque el script no active DTR) y en el arranque hace la TARA sola. Antes de
iniciar el script, dejar el brazo apenas apoyado como para tarar, o tarar
despues con "u t".

TACOMETRO: si en el UNO esta cargado tacometro_lm393.ino en lugar de la
celda, el script lo detecta solo (por el encabezado "tiempo_ms,rpm,..." o por
las lineas de un solo numero) y deja de registrar la balanza: guarda las rpm
en <base>_taco.csv, las pone en el combinado y las muestra en pantalla. Para
fijar el modo sin deteccion: --modo-uno celda | taco (por defecto auto).

Mientras corre, el script tiene los dos puertos abiertos: el monitor serie
del IDE no se puede usar. Los comandos se escriben aca y se pulsa Enter:
    v 60, s, z, m 5, ?  ...  -> van al Pro Micro (los mismos de siempre)
    i 1 / i -2 / i 1 20      -> PUNTO DE 30 s (Ensayo 1): el script usa el modo
                                sostenido "c" del Pro Micro, manda "w" cada 2 s y
                                corta con "s" a los 30 s de corriente estable (o a
                                los segundos indicados, 1 a 120). El tope de 8 s
                                del firmware no aplica. "s" lo corta antes; el
                                resumen "# fin de punto" (I media, duty, V_est)
                                queda en eventos.txt.
    u t, u ?, u c1 1.0 ...   -> "u " adelante: van al UNO (celda)
    salir                    -> apaga el driver (envia s) y termina
Ctrl+C tambien envia "s" al Pro Micro antes de cerrar.

R0 GUIADO (Ensayo 6, limite termico): se maneja en la PC, no va al Pro Micro.
    r0 2        tres puntos de 2 A con el rotor trabado ("r0 -2" al reves).
    r0 4 3      igual, con puntos de 3 s (usar 3 s desde unos 3 A: calienta).
    lv 1.523    tension leida en el multimetro cuando el script la pide.
    r0 cancelar corta la secuencia ("s" tambien la corta y apaga el driver).
En cada intento: el script envia "t" e "i", avisa LEER EL MULTIMETRO AHORA
cuando la corriente esta estable, toma la corriente media del resumen
"# fin de punto" y pide la tension con "lv". Entre intentos espera
PAUSA_R0_S (60 s). Al final muestra R0, la dispersion y los valores para la
hoja Ensayo6-Etapa1, y los guarda en <base>_r0.csv. R = (V - Vesc) / I.
Sin el UNO conectado: agregar --sin-uno.

ENSAYO TERMICO GUIADO (Ensayo 6). Necesita ensayo1_rotor_bloqueado.ino con el
modo sostenido ("c"). Solo mantiene la corriente: la tension en bornes, la T de
carcasa, Ta y R0 se anotan directo en la hoja Ensayo6-Etapa1.
    termico 2      2 A constantes, sin cortes, hasta que se escriba "s"
    termico cancelar / s   termina el ensayo y apaga el driver
Cada 2 min (t = 0, 2, 4 ... min) pita y avisa LECTURA: leer V y T de carcasa y
anotarlas en el Excel. NO hay corte automatico por temperatura (el script no
conoce V ni T): cortar con "s" si la carcasa llega a 70 C o la hoja dice CORTAR.
Mientras dura, manda "w" cada 2 s al Pro Micro: si el script se cae, el Pro
Micro se apaga solo a los 10 s (y a los 60 min como tope de seguridad). El
aviso de LECTURA no toca el motor: la corriente sigue constante. Al final
guarda <base>_termico.csv con la corriente y el duty de cada lectura (la
columna I va a la hoja) y duty/duty(t=0), que sube como R/R0 si Vesc es chica.

Archivos que deja en ../registros/ (nombre con fecha y hora de inicio):
    <base>_micro.csv      cada linea de datos del Pro Micro + hora de la PC
    <base>_celda.csv      cada linea de datos de la celda + hora de la PC
    <base>_taco.csv       cada linea del tacometro + hora de la PC
                          (celda y taco: solo se crean si llegan esos datos)
    <base>_combinado.csv  una fila por linea del Pro Micro con la ultima
                          muestra de la celda o del tacometro al lado (y su
                          antiguedad en ms)
    <base>_eventos.txt    mensajes "#" de las dos placas y comandos enviados
Columnas de tiempo: pc_hora (hora local, para leer) y pc_s (segundos desde el
inicio del registro, para graficar). Las columnas originales de cada placa
(incluido su tiempo_ms) se conservan.
"""

import argparse
import csv
import datetime as dt
import os
import queue
import re
import sys
import threading
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    sys.exit("Falta pyserial: ejecutar  pip install pyserial")

BAUD = 115200

# VID:PID conocidos. Pro Micro / Leonardo: Arduino 2341:8036, SparkFun 1B4F:9206.
# UNO: original 2341:0043/0001, clones con CH340 1A86:7523.
VID_PID_MICRO = {(0x2341, 0x8036), (0x2341, 0x0036), (0x1B4F, 0x9206), (0x1B4F, 0x9205)}
VID_PID_UNO = {(0x2341, 0x0043), (0x2341, 0x0001), (0x1A86, 0x7523), (0x2A03, 0x0043)}

CAMPOS_MICRO = ["tiempo_ms", "estado", "i_obj_A", "i_med_A", "duty_pct", "v_est_V", "t_punto_s"]
CAMPOS_CELDA = ["muestra", "tiempo_ms", "cuentas", "masa_kg", "fuerza_N"]
# Encabezado de tacometro_lm393.ino. Hoy imprime solo las rpm (una por linea);
# si se descomentan las demas columnas, llegan estas cinco.
CAMPOS_TACO = ["tiempo_ms", "rpm", "frecuencia_hz", "pulsos", "rechazados"]

T0 = time.perf_counter()
T0_PC = dt.datetime.now()
bloqueo = threading.Lock()
ultima_celda = None  # (pc_s, campos)
ultimo_taco = None   # (pc_s, campos con el formato de CAMPOS_TACO)
terminar = threading.Event()

# R0 guiado (Ensayo 6)
PAUSA_R0_S = 60.0      # entre intentos, para que el poco calor se reparta
INTENTOS_R0 = 3
DISPERSION_R0_MAX = 2.0  # %; mas que eso: repetir
RE_FIN_PUNTO = re.compile(r"# fin de punto \((?P<motivo>[^)]*)\).*?I_media=(?P<i>-?[\d.]+)")

# Ensayo termico (Ensayo 6)
PERIODO_LECTURA_S = 120.0
PERIODO_VIVO_S = 2.0
DURACION_PUNTO_S = 30.0  # "i <A>": segundos a corriente estable (Ensayo 1)
# Configuracion del R0 guiado, la cargan los comandos vesc / r0
config = {"vesc": 0.0, "r0_filas": None, "r0_amperes": None}


def r0_de_config():
    """R0 = promedio de (V0 - Vesc)/I0 de la ultima medicion r0 (o None)."""
    filas = config["r0_filas"]
    if not filas:
        return None
    return sum((v - config["vesc"]) / i for _, v, i, _ in filas) / len(filas)


def ahora():
    """Hora de la PC: (texto hh:mm:ss.mmm, segundos desde el inicio)."""
    s = time.perf_counter() - T0
    hora = T0_PC + dt.timedelta(seconds=s)
    return hora.strftime("%H:%M:%S.%f")[:-3], s


def buscar_puertos():
    micro = uno = None
    for p in serial.tools.list_ports.comports():
        par = (p.vid, p.pid)
        if par in VID_PID_MICRO and micro is None:
            micro = p.device
        elif par in VID_PID_UNO and uno is None:
            uno = p.device
    return micro, uno


def abrir(puerto, resetear):
    s = serial.Serial()
    s.port = puerto
    s.baudrate = BAUD
    s.timeout = 0.2
    if not resetear:
        # Intenta abrir sin DTR para no reiniciar el UNO. Con el CH340 en
        # Windows igual se reinicia (comprobado 2026-11-14): ver el aviso de
        # la tara en la descripcion.
        s.dtr = False
        s.rts = False
    s.open()
    return s


def es_numero(t):
    try:
        float(t)
        return True
    except ValueError:
        return False


class Registro:
    def __init__(self, carpeta, base):
        os.makedirs(carpeta, exist_ok=True)
        ruta = os.path.join(carpeta, base)
        self.f_micro = open(ruta + "_micro.csv", "w", newline="", encoding="utf-8")
        self.f_comb = open(ruta + "_combinado.csv", "w", newline="", encoding="utf-8")
        self.f_ev = open(ruta + "_eventos.txt", "w", encoding="utf-8")
        self.w_micro = csv.writer(self.f_micro)
        self.w_comb = csv.writer(self.f_comb)
        self.w_micro.writerow(["pc_hora", "pc_s"] + CAMPOS_MICRO)
        self.w_comb.writerow(["pc_hora", "pc_s"] + CAMPOS_MICRO +
                             ["celda_masa_kg", "celda_fuerza_N", "celda_antiguedad_ms", "celda_muestra",
                              "rpm", "rpm_antiguedad_ms"])
        # Celda y tacometro se abren con el primer dato (el UNO tiene uno u otro).
        self.uno = {}  # "celda"/"taco" -> (archivo, escritor)
        self.ruta = ruta

    def escribir_uno(self, tipo, fila):
        """Escribe una fila en <base>_celda.csv o <base>_taco.csv. Llamar con
        el bloqueo tomado."""
        if tipo not in self.uno:
            f = open(f"{self.ruta}_{tipo}.csv", "w", newline="", encoding="utf-8")
            w = csv.writer(f)
            w.writerow(["pc_hora", "pc_s"] + (CAMPOS_CELDA if tipo == "celda" else CAMPOS_TACO))
            self.uno[tipo] = (f, w)
        f, w = self.uno[tipo]
        w.writerow(fila)
        f.flush()

    def evento(self, origen, texto):
        hora, s = ahora()
        with bloqueo:
            self.f_ev.write(f"{hora}\t{s:.3f}\t{origen}\t{texto}\n")
            self.f_ev.flush()
        print(f"[{hora}] {origen}: {texto}")

    def cerrar(self):
        archivos = [self.f_micro, self.f_comb, self.f_ev] + [f for f, _ in self.uno.values()]
        for f in archivos:
            try:
                f.close()
            except OSError:
                pass


def leer_micro(puerto, reg, estado_pantalla):
    while not terminar.is_set():
        try:
            crudo = puerto.readline()
        except serial.SerialException as e:
            reg.evento("PC", f"se perdio el Pro Micro: {e}")
            terminar.set()
            return
        if not crudo:
            continue
        linea = crudo.decode("ascii", errors="replace").strip()
        if not linea:
            continue
        if linea.startswith("#"):
            reg.evento("MICRO", linea)
            for guia in (r0_activo, termico_activo, punto_activo):
                if guia is not None:
                    guia.mensaje(linea)
            continue
        # "562060,R,0.00,0.05,0.00,0.00,0.0  Imed="0.05"": el Imed se descarta
        # (repite i_med_A).
        campos = linea.split()[0].split(",")
        if len(campos) != len(CAMPOS_MICRO) or not es_numero(campos[0]):
            continue  # encabezado u otra cosa
        hora, s = ahora()
        with bloqueo:
            reg.w_micro.writerow([hora, f"{s:.3f}"] + campos)
            reg.f_micro.flush()
            celda = ultima_celda
            if celda is not None:
                c_s, c = celda
                extra = [c[3], c[4], f"{(s - c_s) * 1000:.0f}", c[0]]
            else:
                extra = ["", "", "", ""]
            taco = ultimo_taco
            if taco is not None:
                t_s, t = taco
                extra += [t[1], f"{(s - t_s) * 1000:.0f}"]
            else:
                extra += ["", ""]
            reg.w_comb.writerow([hora, f"{s:.3f}"] + campos + extra)
            reg.f_comb.flush()
            estado_pantalla["micro"] = campos
            estado_pantalla["hora"] = hora
            hist = estado_pantalla.setdefault("hist", [])
            hist.append((s, campos))
            if len(hist) > 200:
                del hist[:100]


def leer_uno(puerto, reg, estado_pantalla, modo_fijo):
    """Lee el UNO, que tiene cargado celda_hx711.ino o tacometro_lm393.ino.
    modo_fijo: "celda", "taco" o None (detectar por el encabezado y el formato)."""
    global ultima_celda, ultimo_taco
    modo = modo_fijo

    def cambiar_modo(nuevo):
        nonlocal modo
        global ultima_celda, ultimo_taco
        if nuevo == modo:
            return
        modo = nuevo
        # Sin datos viejos del otro programa en el combinado ni en pantalla.
        with bloqueo:
            if nuevo == "taco":
                ultima_celda = None
                estado_pantalla.pop("celda", None)
            else:
                ultimo_taco = None
                estado_pantalla.pop("taco", None)
        reg.evento("PC", "UNO con tacometro: se registran rpm (la celda no)" if nuevo == "taco"
                   else "UNO con celda: se registra la fuerza")

    while not terminar.is_set():
        try:
            crudo = puerto.readline()
        except serial.SerialException as e:
            reg.evento("PC", f"se perdio el UNO: {e}")
            return
        if not crudo:
            continue
        linea = crudo.decode("ascii", errors="replace").strip()
        if not linea:
            continue
        if linea.startswith("#"):
            reg.evento("CELDA", linea)
            continue
        campos = [c.strip() for c in linea.split(",")]
        # Los encabezados dicen que programa tiene el UNO (lo imprime al arrancar,
        # y al abrir el puerto el UNO se reinicia).
        if campos == CAMPOS_CELDA or campos == CAMPOS_TACO:
            if modo_fijo is None:
                cambiar_modo("celda" if campos == CAMPOS_CELDA else "taco")
            continue
        if not all(es_numero(c) for c in campos):
            continue  # otra cosa
        if len(campos) == 1:
            # tacometro_lm393.ino tal como esta: solo las rpm.
            if modo_fijo == "celda":
                continue
            cambiar_modo("taco")
            campos = ["", campos[0], "", "", ""]
        elif len(campos) == len(CAMPOS_CELDA):
            # Celda y tacometro completo tienen 5 columnas: decide el encabezado
            # (o --modo-uno); sin ninguno de los dos se toma la celda, como antes.
            if modo is None:
                cambiar_modo("celda")
        else:
            continue
        hora, s = ahora()
        with bloqueo:
            reg.escribir_uno(modo, [hora, f"{s:.3f}"] + campos)
            if modo == "taco":
                ultimo_taco = (s, campos)
                estado_pantalla["taco"] = campos
            else:
                ultima_celda = (s, campos)
                estado_pantalla["celda"] = campos


class Cancelado(Exception):
    pass


r0_activo = None  # GuiaR0 en curso (leer_micro le pasa los mensajes "#")
termico_activo = None  # GuiaTermica en curso
punto_activo = None  # GuiaPunto en curso ("i <A>" de 30 s)


class GuiaR0(threading.Thread):
    """Mide R0 en frio con el rotor trabado: INTENTOS_R0 puntos cortos con el
    comando "i" del Pro Micro; la tension la tipea el usuario con "lv"."""

    def __init__(self, reg, enviar, estado_pantalla, amperes, t_punto):
        super().__init__(daemon=True)
        self.reg, self.enviar, self.pantalla = reg, enviar, estado_pantalla
        self.amperes, self.t_punto = amperes, t_punto
        self.mensajes = queue.Queue()
        self.tensiones = queue.Queue()
        self.cancelado = threading.Event()
        self.esperando_lv = False

    # --- entradas desde los otros hilos ---
    def mensaje(self, linea):
        self.mensajes.put(linea)

    def tension(self, v):
        self.tensiones.put(v)

    def cancelar(self):
        self.cancelado.set()

    # --- utilidades ---
    def _di(self, texto):
        self.reg.evento("R0", texto)

    def _estado(self):
        m = self.pantalla.get("micro")
        return m[1] if m else None

    def _chequear(self):
        if self.cancelado.is_set() or terminar.is_set():
            raise Cancelado()

    def _esperar_estado(self, estados, timeout):
        fin = time.monotonic() + timeout
        while time.monotonic() < fin:
            self._chequear()
            e = self._estado()
            if e in estados:
                return e
            if e == "F":
                raise RuntimeError("el Pro Micro paso a FALLA (ver el mensaje # FALLA); enviar r y repetir")
            time.sleep(0.05)
        raise RuntimeError(f"el Pro Micro no llego al estado {'/'.join(estados)} en {timeout:.0f} s")

    def _esperar_fin_punto(self, timeout):
        fin = time.monotonic() + timeout
        while time.monotonic() < fin:
            self._chequear()
            try:
                linea = self.mensajes.get(timeout=0.1)
            except queue.Empty:
                continue
            if linea.startswith("# FALLA"):
                raise RuntimeError("el Pro Micro paso a FALLA durante el punto; enviar r y repetir")
            if linea.startswith("# fin de punto"):
                m = RE_FIN_PUNTO.search(linea)
                if "a mano" in linea:
                    raise Cancelado()
                if not m:
                    raise RuntimeError("el punto termino sin tramo estable (sin I_media); repetir")
                return float(m.group("i"))
        raise RuntimeError("no llego el resumen '# fin de punto'")

    def _pedir_tension(self, k):
        while not self.tensiones.empty():
            self.tensiones.get_nowait()
        self.esperando_lv = True
        self._di(f"intento {k}: escribir  lv <tension leida>  (ej. lv 1.523)")
        try:
            while True:
                self._chequear()
                try:
                    return self.tensiones.get(timeout=0.2)
                except queue.Empty:
                    pass
        finally:
            self.esperando_lv = False

    # --- secuencia ---
    def run(self):
        global r0_activo
        try:
            self._secuencia()
        except Cancelado:
            self._di("R0 cancelado")
        except RuntimeError as e:
            self._di(f"R0 interrumpido: {e}")
        finally:
            r0_activo = None

    def _secuencia(self):
        a, t = self.amperes, self.t_punto
        self._di(f"inicio: {INTENTOS_R0} puntos de {a:g} A y {t:g} s, pausa de {PAUSA_R0_S:.0f} s. "
                 "Rotor TRABADO y frio, multimetro en V DC en los bornes del motor.")
        self._esperar_estado({"R"}, 130)
        self.enviar(f"t {t:g}")
        time.sleep(0.3)
        filas = []
        for k in range(1, INTENTOS_R0 + 1):
            self._esperar_estado({"R"}, 130)
            while not self.mensajes.empty():
                self.mensajes.get_nowait()
            self.enviar(f"i {a:g}")
            self._esperar_estado({"M"}, 6)
            self._di(f">>> LEER EL MULTIMETRO AHORA (intento {k}) <<<")
            print("\a", end="", flush=True)
            i_media = self._esperar_fin_punto(t + 6)
            v = self._pedir_tension(k)
            r = (abs(v) - config["vesc"]) / abs(i_media)
            filas.append((k, abs(v), abs(i_media), r))
            self._di(f"intento {k}: V={abs(v):.3f} V  I={abs(i_media):.3f} A  R={r:.4f} ohm")
            if k < INTENTOS_R0:
                fin = time.monotonic() + PAUSA_R0_S
                ultimo = None
                while (falta := fin - time.monotonic()) > 0:
                    self._chequear()
                    paso = int(falta // 15)
                    if paso != ultimo:
                        ultimo = paso
                        self._di(f"pausa: faltan {falta:.0f} s para el intento {k + 1}")
                    time.sleep(0.2)
        rs = [f[3] for f in filas]
        r0 = sum(rs) / len(rs)
        disp = (max(rs) - min(rs)) / r0 * 100
        veredicto = "OK" if disp <= DISPERSION_R0_MAX else "REPETIR R0 (revisar contacto de las puntas)"
        config["r0_filas"] = filas
        config["r0_amperes"] = abs(a)
        self._di(f"R0 = {r0:.4f} ohm, dispersion {disp:.1f} % -> {veredicto}")
        self._di("para la hoja Ensayo6-Etapa1 (V0 ; I0): " +
                 "  |  ".join(f"{f[1]:.3f} ; {f[2]:.3f}" for f in filas))
        ruta = self.reg.ruta + "_r0.csv"
        nuevo = not os.path.exists(ruta)
        with open(ruta, "a", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            if nuevo:
                w.writerow(["pc_hora", "i_pedida_A", "intento", "v0_V", "i0_A", "r_ohm", "r0_ohm", "dispersion_pct"])
            hora, _ = ahora()
            for k, v, i, r in filas:
                w.writerow([hora, f"{a:g}", k, f"{v:.3f}", f"{i:.3f}", f"{r:.4f}", f"{r0:.4f}", f"{disp:.2f}"])


class GuiaTermica(threading.Thread):
    """Ensayo 6: corriente sostenida con el modo "c" del Pro Micro y aviso de
    lectura cada 2 min. V y T de carcasa se anotan en el Excel, no aca."""

    def __init__(self, reg, enviar, estado_pantalla, amperes):
        super().__init__(daemon=True)
        self.reg, self.enviar, self.pantalla = reg, enviar, estado_pantalla
        self.amperes = amperes
        self.mensajes = queue.Queue()
        self.cancelado = threading.Event()
        self.filas = []          # dicts por lectura

    # --- entradas ---
    def mensaje(self, linea):
        self.mensajes.put(linea)

    def cancelar(self):
        self.cancelado.set()

    def _di(self, texto):
        self.reg.evento("TERMICO", texto)

    def _estado(self):
        m = self.pantalla.get("micro")
        return m[1] if m else None

    def _revisar_micro(self):
        """Procesa los mensajes del Pro Micro; error si el punto termino."""
        while True:
            try:
                linea = self.mensajes.get_nowait()
            except queue.Empty:
                return
            if linea.startswith("# FALLA"):
                raise RuntimeError(f"el Pro Micro paso a FALLA: {linea[2:]}")
            if linea.startswith("# fin de punto"):
                raise RuntimeError(f"el Pro Micro termino el punto: {linea[2:]}")

    def _chequear(self):
        if self.cancelado.is_set() or terminar.is_set():
            raise Cancelado()
        self._revisar_micro()

    def _promedio_reciente(self, segundos=5.0):
        """(I media, duty medio) de las lineas de los ultimos segundos."""
        with bloqueo:
            hist = list(self.pantalla.get("hist", []))
        if not hist:
            return None, None
        ultimo = hist[-1][0]
        sel = [c for s, c in hist if s >= ultimo - segundos and c[1] == "M"]
        if not sel:
            return None, None
        i = sum(abs(float(c[3])) for c in sel) / len(sel)
        d = sum(float(c[4]) for c in sel) / len(sel)
        return i, d

    def _vivo(self):
        """Manda "w" cada PERIODO_VIVO_S mientras dure el ensayo."""
        while not self.cancelado.is_set() and not terminar.is_set() and self.is_alive():
            self.enviar("w", silencioso=True)
            time.sleep(PERIODO_VIVO_S)

    # --- secuencia ---
    def run(self):
        global termico_activo
        try:
            self._secuencia()
        except Cancelado:
            self.enviar("s")
            self._di("ensayo terminado con 's', driver apagado")
        except RuntimeError as e:
            self.enviar("s")
            self._di(f"ensayo interrumpido: {e}")
        finally:
            self._resumen()
            termico_activo = None

    def _secuencia(self):
        a = self.amperes
        self._di(f"inicio: {a:g} A constantes hasta 's'. SIN corte automatico por temperatura: "
                 f"cortar con 's' si la carcasa llega a 70 C o la hoja marca CORTAR.")
        if self._estado() != "R":
            raise RuntimeError("el Pro Micro no esta en reposo (R); esperar o enviar r")
        while not self.mensajes.empty():
            self.mensajes.get_nowait()
        self.enviar(f"c {a:g}")
        threading.Thread(target=self._vivo, daemon=True).start()
        fin = time.monotonic() + 6
        while self._estado() != "M":
            self._chequear()
            if time.monotonic() > fin:
                raise RuntimeError("no llego a corriente estable en 6 s (revisar que el sketch tenga el modo c)")
            time.sleep(0.05)
        t0 = time.monotonic()
        k = 0
        while True:  # termina solo con "s" (Cancelado) o si el Pro Micro corta
            t_obj = k * PERIODO_LECTURA_S
            while time.monotonic() - t0 < t_obj:
                self._chequear()
                time.sleep(0.1)
            self._lectura(t_obj / 60.0)
            k += 1

    def _lectura(self, t_min):
        """Avisa la lectura y guarda I y duty del momento."""
        i, d = self._promedio_reciente()
        d0 = self.filas[0]["duty"] if self.filas else d
        rel = d / d0 if d and d0 else None
        self.filas.append({"t_min": t_min, "i": i, "duty": d, "rel": rel})
        print("\a", end="", flush=True)
        self._di(f">>> LECTURA t={t_min:g} min: anotar V en bornes y T carcasa en el Excel <<<  "
                 + (f"I={i:.3f} A" if i else "I=?") + "  "
                 + (f"duty={d:.2f} %" if d else "duty=?")
                 + (f"  duty/duty0={rel:.3f}" if rel else ""))

    def _resumen(self):
        if not self.filas:
            return
        ruta = self.reg.ruta + "_termico.csv"
        with open(ruta, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(["t_min", "i_A", "duty_pct", "duty_rel"])
            for x in self.filas:
                w.writerow([f"{x['t_min']:g}",
                            f"{x['i']:.3f}" if x["i"] else "",
                            f"{x['duty']:.2f}" if x["duty"] else "",
                            f"{x['rel']:.4f}" if x["rel"] else ""])
        self._di(f"tabla guardada en {ruta} (columna i_A -> columna I de la hoja)")


class GuiaPunto(GuiaTermica):
    """Ensayo 1: "i <A> [s]" mantiene la corriente DURACION_PUNTO_S (o los
    segundos indicados) con el modo "c" del Pro Micro y la corta sola con "s".
    El tope de 8 s del punto "i" del firmware no aplica."""

    def __init__(self, reg, enviar, estado_pantalla, amperes, segundos):
        super().__init__(reg, enviar, estado_pantalla, amperes)
        self.segundos = segundos
        self.completo = False

    def _di(self, texto):
        self.reg.evento("PUNTO", texto)

    def run(self):
        global punto_activo
        try:
            self._secuencia()
        except Cancelado:
            self._di("punto cortado con 's' antes de tiempo")
        except RuntimeError as e:
            self._di(f"punto interrumpido: {e}")
        finally:
            self.enviar("s")  # el resumen "# fin de punto" lo imprime el Pro Micro
            if self.completo:
                self._di(f"punto terminado: {self.amperes:g} A durante {self.segundos:g} s, driver apagado")
            punto_activo = None

    def _secuencia(self):
        a, seg = self.amperes, self.segundos
        if self._estado() != "R":
            raise RuntimeError("el Pro Micro no esta en reposo (R); esperar la pausa o enviar r")
        while not self.mensajes.empty():
            self.mensajes.get_nowait()
        self._di(f"inicio: {a:g} A durante {seg:g} s (modo sostenido; 's' lo corta antes)")
        self.enviar(f"c {a:g}")
        threading.Thread(target=self._vivo, daemon=True).start()
        fin = time.monotonic() + 6
        while self._estado() != "M":
            self._chequear()
            if time.monotonic() > fin:
                raise RuntimeError("no llego a corriente estable en 6 s (revisar que el sketch tenga el modo c)")
            time.sleep(0.05)
        fin = time.monotonic() + seg  # los segundos cuentan desde que la corriente es estable
        while time.monotonic() < fin:
            self._chequear()
            time.sleep(0.05)
        self.completo = True
        self.cancelado.set()  # corta el hilo de "w"


def mostrar(estado_pantalla):
    """Una linea de resumen cada 0,5 s en la consola."""
    while not terminar.is_set():
        time.sleep(0.5)
        m = estado_pantalla.get("micro")
        c = estado_pantalla.get("celda")
        t = estado_pantalla.get("taco")
        partes = [estado_pantalla.get("hora", "--:--:--")]
        if m:
            partes.append(f"{m[1]} I_obj={m[2]} I={m[3]} A duty={m[4]} % t={m[6]} s")
        else:
            partes.append("Pro Micro sin datos")
        if t:
            partes.append(f"{t[1]} rpm")
        elif c:
            partes.append(f"F={c[4]} N ({c[3]} kg)")
        else:
            partes.append("UNO sin datos")
        print("  ".join(partes))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--micro", help="puerto del Pro Micro (ej. COM4)")
    ap.add_argument("--uno", help="puerto del UNO con la celda (ej. COM12)")
    ap.add_argument("--nombre", default="ensayo1", help="prefijo de los archivos")
    ap.add_argument("--carpeta", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "registros"))
    ap.add_argument("--resetear-uno", action="store_true",
                    help="reinicia el UNO al abrir (hace la tara de arranque)")
    ap.add_argument("--modo-uno", choices=["auto", "celda", "taco"], default="auto",
                    help="programa cargado en el UNO (auto: lo detecta por lo que imprime)")
    ap.add_argument("--silencioso", action="store_true", help="sin la linea de resumen cada 0,5 s")
    ap.add_argument("--sin-uno", action="store_true", help="solo el Pro Micro (por ejemplo, Ensayo 6)")
    args = ap.parse_args()

    auto_micro, auto_uno = buscar_puertos()
    p_micro = args.micro or auto_micro
    p_uno = None if args.sin_uno else (args.uno or auto_uno)
    if not p_micro or (not p_uno and not args.sin_uno):
        print("Puertos encontrados:")
        for p in serial.tools.list_ports.comports():
            print(f"  {p.device}  {p.description}")
        sys.exit("No se encontro el Pro Micro o el UNO: indicar --micro COMx --uno COMy "
                 "(o --sin-uno si el UNO no se usa)")

    try:
        micro = abrir(p_micro, resetear=True)  # el Pro Micro no se reinicia al abrir
        uno = abrir(p_uno, resetear=args.resetear_uno) if p_uno else None
    except serial.SerialException as e:
        sys.exit(f"No se pudo abrir un puerto ({e}). Cerrar el monitor serie del IDE.")

    base = f"{args.nombre}_{T0_PC.strftime('%Y%m%d_%H%M%S')}"
    reg = Registro(os.path.abspath(args.carpeta), base)
    reg.evento("PC", f"inicio del registro: Pro Micro {p_micro}, UNO {p_uno or 'no usado'}, "
                     f"archivos {reg.ruta}_*")

    estado_pantalla = {}
    hilos = [threading.Thread(target=leer_micro, args=(micro, reg, estado_pantalla), daemon=True)]
    if uno is not None:
        hilos.append(threading.Thread(target=leer_uno, daemon=True,
                                      args=(uno, reg, estado_pantalla,
                                            None if args.modo_uno == "auto" else args.modo_uno)))
    if not args.silencioso:
        hilos.append(threading.Thread(target=mostrar, args=(estado_pantalla,), daemon=True))
    for h in hilos:
        h.start()

    envio_lock = threading.Lock()

    def enviar_micro(texto, silencioso=False):
        try:
            with envio_lock:
                micro.write((texto + "\n").encode("ascii", errors="replace"))
            if not silencioso:
                reg.evento("PC", f"comando a MICRO: {texto}")
        except serial.SerialException as e:
            reg.evento("PC", f"no se pudo enviar a MICRO: {e}")

    print("Comandos: i/v/s/z/m/?... al Pro Micro; 'u <cmd>' al UNO; 'r0 <A>' R0 guiado; "
          "'vesc', 'termico <A>' Ensayo 6; 'salir' para terminar.")
    global r0_activo, termico_activo, punto_activo
    try:
        for linea in sys.stdin:
            cmd = linea.strip()
            if terminar.is_set():
                break
            if not cmd:
                continue
            low = cmd.lower()
            if low == "salir":
                break
            # ---- R0 guiado: se resuelve aca, nunca va al Pro Micro ----
            partes = low.replace(",", ".").split()
            # ---- configuracion y ensayo termico (Ensayo 6) ----
            if partes[0] in ("ta", "lt") or partes[0].startswith("r0="):
                print(f"'{partes[0]}' ya no se usa: Ta, R0, V y T de carcasa se anotan directo en la hoja "
                      "Ensayo6-Etapa1. (No se envia al Pro Micro.)")
                continue
            if partes[0] == "vesc":
                try:
                    valor = float(partes[1])
                except (IndexError, ValueError):
                    print("Uso: vesc <V>  (ej. vesc 0.8)")
                    continue
                if termico_activo is not None:
                    print("No se cambia durante el ensayo termico.")
                    continue
                config[partes[0]] = valor
                r0 = r0_de_config()
                nota = f"  (R0 recalculado = {r0:.4f} ohm)" if r0 else ""
                reg.evento("PC", f"{partes[0]} = {valor:g}" + nota)
                continue
            if partes[0] == "termico":
                if len(partes) > 1 and partes[1] == "cancelar":
                    if termico_activo is not None:
                        termico_activo.cancelar()
                    else:
                        print("No hay un ensayo termico en curso.")
                    continue
                if termico_activo is not None or r0_activo is not None or punto_activo is not None:
                    print("Ya hay un ensayo guiado en curso.")
                    continue
                try:
                    amperes = float(partes[1])
                    if not 0.1 <= abs(amperes) <= 6.0:
                        raise ValueError
                except (IndexError, ValueError):
                    print("Uso: termico <A>   ej. 'termico 2' (2 A constantes hasta 's')")
                    continue
                termico_activo = GuiaTermica(reg, enviar_micro, estado_pantalla, amperes)
                termico_activo.start()
                continue
            if termico_activo is not None and partes[0] not in ("s", "?") and not low.startswith("u "):
                print("Ensayo termico en curso: solo 's', '?', 'termico cancelar' o 'u <cmd>'.")
                continue
            if termico_activo is not None and partes[0] == "s":
                termico_activo.cancelar()
            # ---- punto de Ensayo 1: "i <A> [s]" sostenido DURACION_PUNTO_S ----
            if punto_activo is not None:
                if partes[0] == "s":
                    punto_activo.cancelar()  # y el "s" sigue hacia el Pro Micro
                elif partes[0] != "?" and not low.startswith("u "):
                    print("Punto en curso: solo 's', '?' o 'u <cmd>'.")
                    continue
            elif partes[0] == "i" and r0_activo is None and termico_activo is None:
                try:
                    amperes = float(partes[1])
                    segundos = float(partes[2]) if len(partes) > 2 else DURACION_PUNTO_S
                    if not (0.1 <= abs(amperes) <= 5.0 and 1.0 <= segundos <= 120.0):
                        raise ValueError
                except (IndexError, ValueError):
                    print(f"Uso: i <A> [s]   ej. 'i 1' ({DURACION_PUNTO_S:g} s) o 'i -2 10' "
                          "(0,1 a 5 A; 1 a 120 s)")
                    continue
                punto_activo = GuiaPunto(reg, enviar_micro, estado_pantalla, amperes, segundos)
                punto_activo.start()
                continue
            if partes[0] == "r0":
                if len(partes) > 1 and partes[1] == "cancelar":
                    if r0_activo is not None:
                        r0_activo.cancelar()
                        enviar_micro("s")
                    else:
                        print("No hay un R0 en curso.")
                    continue
                if r0_activo is not None:
                    print("Ya hay un R0 en curso ('r0 cancelar' para cortarlo).")
                    continue
                if termico_activo is not None or punto_activo is not None:
                    print("Ya hay un ensayo guiado en curso.")
                    continue
                try:
                    amperes = float(partes[1])
                    t_punto = float(partes[2]) if len(partes) > 2 else 5.0
                    if not (0.1 <= abs(amperes) <= 5.0 and 1.0 <= t_punto <= 8.0):
                        raise ValueError
                except (IndexError, ValueError):
                    print("Uso: r0 <A> [s]   ej. 'r0 2' o 'r0 4 3' (0,1 a 5 A; 1 a 8 s por punto)")
                    continue
                r0_activo = GuiaR0(reg, enviar_micro, estado_pantalla, amperes, t_punto)
                r0_activo.start()
                continue
            if partes[0] == "lv":
                guia = r0_activo
                try:
                    valor = float(partes[1])
                except (IndexError, ValueError):
                    print("Uso: lv <tension en V>   ej. lv 1.523")
                    continue
                if guia is None or not guia.esperando_lv:
                    print("Ahora no se espera ninguna tension (lv solo sirve dentro de r0).")
                    continue
                guia.esperando_lv = False  # una sola tension por pedido
                reg.evento("PC", f"tension leida: {valor:.3f} V")
                guia.tension(valor)
                continue
            if r0_activo is not None and not low.startswith("u ") and partes[0] not in ("s", "?"):
                print("R0 en curso: solo 'lv', 's', '?', 'r0 cancelar' o 'u <cmd>'.")
                continue
            if r0_activo is not None and partes[0] == "s":
                r0_activo.cancelar()
            if low.startswith("u "):
                if uno is None:
                    print("El UNO no esta conectado (--sin-uno).")
                    continue
                destino, puerto, texto = "UNO", uno, cmd[2:].strip()
            else:
                destino, puerto, texto = "MICRO", micro, cmd
            try:
                puerto.write((texto + "\n").encode("ascii", errors="replace"))
                reg.evento("PC", f"comando a {destino}: {texto}")
            except serial.SerialException as e:
                reg.evento("PC", f"no se pudo enviar a {destino}: {e}")
    except KeyboardInterrupt:
        pass
    finally:
        # Lo primero al salir: apagar el driver.
        try:
            micro.write(b"s\n")
            micro.flush()
            reg.evento("PC", "comando a MICRO: s (al salir)")
        except serial.SerialException:
            pass
        time.sleep(0.3)
        terminar.set()
        time.sleep(0.3)
        for p in (micro, uno):
            if p is None:
                continue
            try:
                p.close()
            except serial.SerialException:
                pass
        reg.evento("PC", "fin del registro")
        reg.cerrar()


if __name__ == "__main__":
    main()
