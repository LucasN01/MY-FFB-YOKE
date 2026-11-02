#if defined(YOKE_TARGET_RP2350)
// Cerrojos de newlib para el compilador pqt-gcc (GCC 16.1, decision U3).
// Su newlib esta compilada con _RETARGETABLE_LOCKING: espera que el firmware
// defina los objetos __lock___* y las funciones __retarget_lock_*. El core
// Arduino-Pico los daba en cores/rp2040/lock.cpp; el Pico SDK no los trae.
// Se replica ese comportamiento sin FreeRTOS: cada cerrojo es un mutex del SDK
// guardado dentro del struct __lock que reserva newlib.
#include <pico/mutex.h>
#include <sys/lock.h>

static_assert(sizeof(mutex_t) <= sizeof(struct __lock), "mutex_t no entra en struct __lock");
static_assert(sizeof(recursive_mutex_t) <= sizeof(struct __lock),
              "recursive_mutex_t no entra en struct __lock");

// Los objetos globales van en .mutex_array para que runtime_init_mutex los
// inicialice antes de main(); spin_lock = 1 marca un mutex recursivo
// (mismo criterio que auto_init_recursive_mutex del SDK).
#define YOKE_NEWLIB_MUTEX(name) __attribute__((section(".mutex_array"))) mutex_t name
#define YOKE_NEWLIB_RECURSIVE_MUTEX(name)                                               \
  __attribute__((section(".mutex_array"))) recursive_mutex_t name = {                   \
      .core = {.spin_lock = (spin_lock_t*)1}, .owner = 0, .enter_count = 0}

extern "C" {
YOKE_NEWLIB_RECURSIVE_MUTEX(__lock___sinit_recursive_mutex);
YOKE_NEWLIB_RECURSIVE_MUTEX(__lock___sfp_recursive_mutex);
YOKE_NEWLIB_RECURSIVE_MUTEX(__lock___atexit_recursive_mutex);
YOKE_NEWLIB_MUTEX(__lock___at_quick_exit_mutex);
YOKE_NEWLIB_RECURSIVE_MUTEX(__lock___malloc_recursive_mutex);
YOKE_NEWLIB_RECURSIVE_MUTEX(__lock___env_recursive_mutex);
YOKE_NEWLIB_MUTEX(__lock___tz_mutex);
YOKE_NEWLIB_MUTEX(__lock___dd_hash_mutex);
YOKE_NEWLIB_MUTEX(__lock___arc4random_mutex);

void __retarget_lock_init(_LOCK_T lock) { mutex_init((mutex_t*)lock); }
void __retarget_lock_init_recursive(_LOCK_T lock) {
  recursive_mutex_init((recursive_mutex_t*)lock);
}
void __retarget_lock_close(_LOCK_T lock) { (void)lock; }
void __retarget_lock_close_recursive(_LOCK_T lock) { (void)lock; }
void __retarget_lock_acquire(_LOCK_T lock) { mutex_enter_blocking((mutex_t*)lock); }
void __retarget_lock_acquire_recursive(_LOCK_T lock) {
  recursive_mutex_enter_blocking((recursive_mutex_t*)lock);
}
int __retarget_lock_try_acquire(_LOCK_T lock) {
  return mutex_try_enter((mutex_t*)lock, nullptr);
}
int __retarget_lock_try_acquire_recursive(_LOCK_T lock) {
  return recursive_mutex_try_enter((recursive_mutex_t*)lock, nullptr);
}
void __retarget_lock_release(_LOCK_T lock) { mutex_exit((mutex_t*)lock); }
void __retarget_lock_release_recursive(_LOCK_T lock) {
  recursive_mutex_exit((recursive_mutex_t*)lock);
}
}
#endif
