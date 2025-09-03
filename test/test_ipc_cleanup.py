import ctypes, sys

libc = ctypes.CDLL(None)
ftok = libc.ftok; ftok.argtypes = [ctypes.c_char_p, ctypes.c_int]; ftok.restype = ctypes.c_int
shmget = libc.shmget; shmget.argtypes = [ctypes.c_int, ctypes.c_size_t, ctypes.c_int]; shmget.restype = ctypes.c_int
msgget = libc.msgget; msgget.argtypes = [ctypes.c_int, ctypes.c_int]; msgget.restype = ctypes.c_int

shm_key = ftok(b".gitmodules", 130)

shm_exists = shmget(shm_key, 0, 0o666) != -1
msq_exists = msgget(shm_key, 0o666) != -1

if shm_exists or msq_exists:
    print(f"Leftover: shm={shm_exists} msq={msq_exists}")
    sys.exit(1)
else:
    print(f"Successfull cleanup")
sys.exit(0)
