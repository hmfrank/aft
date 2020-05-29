#ifndef IMPLEMENTIERUNG_DEMO_H
#define IMPLEMENTIERUNG_DEMO_H

/// Runs a demonstration program of the algorithm described in
/// [2009 Davies, Plumbley, Stark - Real-time Beat-synchronous Analysis of Musical Audio].
///
/// \return Error Codes:
///	    0: program terminated successfully
///     1: can't parse arguments
///     2: no audio devices detected
int demo(int argc, char **argv);

#endif //IMPLEMENTIERUNG_DEMO_H
