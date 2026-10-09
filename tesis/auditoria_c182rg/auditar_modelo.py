"""Read-only audit of the installed aircraft, current sources and saved telemetry.

Run with Python 3 (standard library only). Writes evidence alongside this script.
Does not connect to Prepar3D, serial ports or motors, and never edits input files.
"""
import csv
import hashlib
import json
import math
import statistics
import struct
import sys
from pathlib import Path

OUT = Path(__file__).resolve().parent
ROOT = OUT.parent.parent
ORIGINAL = Path(r"C:\Users\norie\Downloads\fsxnative_cessna182rg\Cessna182RG_fsx")
INSTALLED = Path(r"C:\Users\norie\OneDrive\Documentos\Prepar3D v4 Add-ons\Cessna182RG_AF_Scrub\SimObjects\Airplanes\Cessna182RG_fsx")


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def number(value):
    try:
        return float(value)
    except (ValueError, TypeError):
        return math.nan


def write_json(name, value):
    (OUT / name).write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False), encoding="utf-8")


def write_csv(name, rows):
    with (OUT / name).open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(rows)


def cfg(path):
    sections = {}
    section = ""
    for line in path.read_text(encoding="cp1252").splitlines():
        line = line.split("//", 1)[0].split(";", 1)[0].strip()
        if line.startswith("[") and line.endswith("]"):
            section = line[1:-1].lower()
            sections.setdefault(section, {})
        elif "=" in line:
            key, value = line.split("=", 1)
            sections.setdefault(section, {})[key.strip().lower()] = value.strip()
    return sections


def main():
    sources = [ROOT / p for p in (
        "SimConnectBridge/main.cpp", "SimConnectBridge/GustModel.h",
        "SimConnectBridge/BuffetModel.h", "SimConnectBridge/GroundModel.h",
        "SimEffects.h", "SimTelemetry.h", "mis_defines.h", "GainSettings.h",
        "YokeMotion.h", "MotorDriver.h", "MiYoke_PD_V2.3.ino", "LcdTelemetry.h",
        "VPforce-TelemFFB-refactor/telemffb/utils/TurbulenceModulator.py",
        "VPforce-TelemFFB-refactor/telemffb/sim/msfs_xp/MsfsXpFlightControlsMixIn.py",
        "VPforce-TelemFFB-refactor/telemffb/sim/base/BuffetingEffectMixIn.py",
        "VPforce-TelemFFB-refactor/telemffb/sim/base/MotionEffectsMixIn.py",
    )]
    sources += [folder / name for folder in (ORIGINAL, INSTALLED) for name in ("aircraft.cfg", "Cessna182.air", "cessna182_ref.txt")]
    hashes = [{"path": str(path), "sha256": sha(path), "bytes": path.stat().st_size} for path in sources]
    original_cfg, installed_cfg = cfg(ORIGINAL / "aircraft.cfg"), cfg(INSTALLED / "aircraft.cfg")
    differences = []
    for section in sorted(set(original_cfg) | set(installed_cfg)):
        a, b = original_cfg.get(section, {}), installed_cfg.get(section, {})
        for key in sorted(set(a) | set(b)):
            if a.get(key) != b.get(key):
                differences.append({"section": section, "key": key, "original": a.get(key), "installed": b.get(key)})

    # Structural extraction only; hexadecimal record IDs are not converted into
    # certified aerodynamic meanings. Record 0x0404 is preserved as raw pairs.
    air = (INSTALLED / "Cessna182.air").read_bytes()
    records, pairs = [], []
    offset = 0
    while offset < len(air):
        if len(air) - offset < 8:
            raise ValueError("Truncated AIR record header")
        token, size = struct.unpack_from("<II", air, offset)
        if offset + 8 + size > len(air):
            raise ValueError("AIR record exceeds file size")
        records.append({"token_hex": f"0x{token:04X}", "offset": offset, "bytes": size})
        if token == 0x0404:
            count, = struct.unpack_from("<I", air, offset + 8)
            if size != 4 + 16 * count:
                raise ValueError("Unexpected 0x0404 layout")
            for i in range(count):
                x, y = struct.unpack_from("<dd", air, offset + 12 + 16 * i)
                pairs.append({"index": i, "x_raw": x, "x_if_radians_degrees": math.degrees(x), "y_raw": y})
        offset += 8 + size
    write_csv("registros_air.csv", records)
    if pairs:
        write_csv("tabla_0404_sin_calibrar.csv", pairs)

    log = ROOT / "SimConnectBridge/bin/Win32/Release/telemetry.csv"
    # Snapshot in memory: no write to the application's telemetry file.
    snapshot = OUT / "telemetry_snapshot.csv"
    if not snapshot.exists() or "--refresh-telemetry" in sys.argv:
        snapshot.write_bytes(log.read_bytes())
    raw = snapshot.read_bytes()
    rows = list(csv.DictReader(raw.decode("utf-8-sig").splitlines()))
    complete = [r for r in rows if None not in r and all(v is not None for v in r.values())]
    fields = {}
    for key in complete[0]:
        vals = [number(r[key]) for r in complete]
        finite = [v for v in vals if math.isfinite(v)]
        fields[key] = {"min": min(finite) if finite else None, "max": max(finite) if finite else None,
                       "invalid": len(vals) - len(finite), "nonzero": sum(v != 0 for v in finite)}
    times = [int(r["time_ms"]) for r in complete]
    dts = [b-a for a, b in zip(times, times[1:])]
    airborne = [r for r in complete if number(r["on_ground"]) == 0 and number(r["armed"]) == 1]
    telemetry = {"path": str(log), "sha256": hashlib.sha256(raw).hexdigest(),
                 "rows": len(complete), "discarded_partial_rows": len(rows) - len(complete),
                 "duration_s": (times[-1] - times[0]) / 1000,
                 "dt_median_ms": statistics.median(dts), "dt_max_ms": max(dts),
                 "duplicate_timestamps": dts.count(0), "airborne_armed_rows": len(airborne),
                 "q_saturated_rows": sum(number(r["q_ratio"]) >= 1 for r in complete), "fields": fields}
    write_json("evidencia.json", {"hashes": hashes, "cfg_differences": differences,
        "air_files_identical": sha(ORIGINAL / "Cessna182.air") == sha(INSTALLED / "Cessna182.air"),
        "installed_cfg": installed_cfg, "air_record_count": len(records), "telemetry": telemetry})

    # Analytic steady-state examples of the checked-in defaults, not measured
    # motor force; 1/2 rho0 V_EAS^2. No trim or motion compensation in this table.
    cases = []
    for speed in (0, 40, 60, 80, 100, 120, 140, 160, 180):
        pressure = 0.5 * 1.225 * (speed * 1852/3600) ** 2
        q = min((speed/120) ** 2, 1)
        cases.append({"EAS_knots": speed, "q_Pa": pressure, "q_ratio": q,
                      "stiffness_fraction": 0.15 + 0.85*q,
                      "pitch_PWM_magnitude_at_half_travel": 255*0.9*0.8*0.5*(0.15 + 0.85*q),
                      "roll_PWM_magnitude_at_half_travel": 255*0.9*0.4*0.5*(0.15 + 0.85*q)})
    write_csv("centrado_actual_ejemplos.csv", cases)
    ground = [{"ground_speed_kt": v, "envelope": max(0, min((v-0.5)/39.5, 1)),
               "peak_roll_pwm_default": min(40, 255*0.9*0.18*max(0, min((v-0.5)/39.5, 1)))} for v in (0, 0.5, 1, 5, 10, 20, 40, 60)]
    write_csv("rodaje_actual_ejemplos.csv", ground)
    print(json.dumps({"air_records": len(records), "cfg_differences": differences,
                      "telemetry": {k: v for k, v in telemetry.items() if k not in ("fields", "path")},
                      "selected_fields": {k: fields[k] for k in ("ias_knots", "aoa_deg", "stall_alpha_deg", "vs0_knots", "vs1_knots", "stall_amp", "spoiler_left", "gear_amp", "ground_amp", "q_pa")}}, indent=2))


if __name__ == "__main__":
    main()
