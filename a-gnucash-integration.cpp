#include <iostream>

#include <gnucash/gnc-engine.h>

using std::cout;
using std::endl;

class GncEngineRAII {
public:
    GncEngineRAII()
    {
        cout << "initializing engine..." << endl;
        gnc_engine_init(0, { NULL });
        while (!gnc_engine_is_initialized()) {
            cout << "waiting for engine to initialize..." << endl;
        }
    };
    ~GncEngineRAII()
    {
        cout << "shutting down engine..." << endl;
        gnc_engine_shutdown();
        while (gnc_engine_is_initialized()) {
            cout << "waiting for engine to shut down..." << endl;
        }
    }
};

int main()
{
    qof_log_init();

    qof_log_set_level ("", QOF_LOG_DEBUG);
    qof_log_set_level ("qof", QOF_LOG_DEBUG);
    qof_log_set_level ("gnc", QOF_LOG_DEBUG);

    GncEngineRAII engine;

    return 0;
}
