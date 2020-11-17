#include <iostream>

#include <gnucash/gnc-engine.h>
#include <gnucash/qofsession.h>

const char* postgresUrl = "postgres://gnc:gnc@127.0.0.1:5432/gnucash";

using std::cout;
using std::endl;

class GncEngineRAII {
public:
    GncEngineRAII()
    {
        cout << "initializing engine..." << endl;
        gnc_engine_init(0, nullptr);
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

    struct QSErr { // qof session error
        QofSession* session; // in an error state
        const char* function; // qof_session_*, that induced the error
    };

    try {
        GncEngineRAII engine;

        auto session { qof_session_new(qof_book_new()) };
        if(qof_session_get_error(session)) throw QSErr { session, "new" };

        qof_session_begin(session, postgresUrl, SessionOpenMode::SESSION_NEW_OVERWRITE);
        if(qof_session_get_error(session)) throw QSErr { session, "begin" };
    } catch(QSErr err) {
        cerr << string{"qof session error message: "} <<
            qof_session_get_error_message(err.session) << "(" <<
            qof_session_get_error(err.session) <<
            "). from function qof_session_" << err.function << endl;
        return 1;
    }

    return 0;
}
