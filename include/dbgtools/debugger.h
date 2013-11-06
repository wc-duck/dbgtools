#ifndef DEBUG_DEBUGGER_H_INCLUDED
#define DEBUG_DEBUGGER_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Return non-zero if a debugger is currently attached to this process.
 *
 * @note this might on some platforms be quite slow.
 */
int debugger_present();

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif // DEBUG_DEBUGGER_H_INCLUDED
