#!/usr/bin/env python3
"""Boot KFS in headless QEMU and check the 80x25 VGA text buffer (same as make run)."""

import argparse
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import time

VGA_PHYS = 0xB8000
VGA_BYTES = 80 * 25 * 2


def die(msg):
    sys.exit("FAIL: " + msg)


def qmp_read(sock):
    sock.settimeout(10)
    buf = b""
    while True:
        chunk = sock.recv(4096)
        if not chunk:
            break
        buf += chunk
        if b"\n" in buf:
            line, buf = buf.split(b"\n", 1)
            obj = json.loads(line.decode())
            if "event" in obj:
                continue
            return obj
    die("QMP: connexion fermee")


def qmp(sock, cmd):
    sock.sendall((json.dumps(cmd) + "\n").encode())
    return qmp_read(sock)


def qmp_hmp(sock, line):
    r = qmp(sock, {
        "execute": "human-monitor-command",
        "arguments": {"command-line": line},
    })
    if "error" in r:
        die("QMP: " + json.dumps(r["error"]))
    return r.get("return", "")


def dump_vga(sock):
    raw = qmp_hmp(sock, "xp /%dxb 0x%x" % (VGA_BYTES, VGA_PHYS))
    vals = [int(m.group(1), 16) for m in re.finditer(r"0x([0-9a-fA-F]{1,2})", raw)]
    if len(vals) < VGA_BYTES:
        return ""
    chars = bytearray(vals[i] for i in range(0, VGA_BYTES, 2))
    return chars.decode("latin1")


def wait_vga(proc, sock, needle, seconds, label):
    deadline = time.time() + seconds
    last = ""
    while time.time() < deadline:
        if proc.poll() is not None:
            die("QEMU s'est arrete pendant %s (code %s)" % (label, proc.returncode))
        last = dump_vga(sock)
        if needle in last:
            return last
        time.sleep(0.2)
    die("%s: pas %r dans l'ecran QEMU apres %ss\n--- ecran ---\n%s"
        % (label, needle, seconds, preview(last)))


def boot_and_check(qemu, extra_args, expect, via_grub):
    sock_path = "/tmp/kfs-qmp.sock"
    try:
        os.unlink(sock_path)
    except OSError:
        pass
    cmd = [
        qemu,
        "-m", "64",
        "-accel", "tcg",
        "-display", "none",
        "-vga", "std",
        "-serial", "none",
        "-no-reboot",
        "-qmp", "unix:%s,server,nowait" % sock_path,
    ] + extra_args
    log_path = "/tmp/kfs-qemu.log"
    log = open(log_path, "wb")
    proc = subprocess.Popen(cmd, stdout=log, stderr=log)
    sock = None
    try:
        deadline = time.time() + 20
        while not os.path.exists(sock_path):
            if proc.poll() is not None:
                die("QEMU n'a pas demarre (code %s), voir %s"
                    % (proc.returncode, log_path))
            if time.time() > deadline:
                die("socket QMP absent")
            time.sleep(0.05)
        time.sleep(0.2)
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(sock_path)
        qmp_read(sock)
        qmp(sock, {"execute": "qmp_capabilities"})
        if via_grub:
            wait_vga(proc, sock, "GRUB", 20, "boot GRUB")
            qmp_hmp(sock, "sendkey ret")
        wait_vga(proc, sock, expect, 20, "boot KFS")
        qmp(sock, {"execute": "quit"})
        proc.wait(timeout=10)
    finally:
        if sock is not None:
            sock.close()
        if proc.poll() is None:
            proc.kill()
        log.close()
        try:
            os.unlink(sock_path)
        except OSError:
            pass


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", required=True)
    ap.add_argument("--iso", required=True)
    ap.add_argument("--expect", default="42")
    args = ap.parse_args()

    qemu = shutil.which("qemu-system-i386")
    if not qemu:
        die("qemu-system-i386 introuvable (brew/apt: qemu)")
    if not os.path.isfile(args.bin) or os.path.getsize(args.bin) == 0:
        die("binaire manquant: " + args.bin)
    if not os.path.isfile(args.iso) or os.path.getsize(args.iso) == 0:
        die("ISO manquante: " + args.iso)

    if shutil.which("grub-file"):
        r = subprocess.run(["grub-file", "--is-x86-multiboot", args.bin])
        if r.returncode != 0:
            die("pas un binaire Multiboot: " + args.bin)
        print("ok: Multiboot")

    print("test: QEMU -kernel (KFS sans menu)")
    boot_and_check(qemu, ["-kernel", os.path.abspath(args.bin)], args.expect, False)
    print("ok: ecran QEMU contient %r" % args.expect)

    print("test: QEMU -cdrom (GRUB puis KFS, comme make run)")
    boot_and_check(
        qemu,
        ["-boot", "d", "-cdrom", os.path.abspath(args.iso)],
        args.expect,
        True,
    )
    print("ok: GRUB + ecran QEMU contient %r" % args.expect)
    print("tous les tests OK")


if __name__ == "__main__":
    main()
