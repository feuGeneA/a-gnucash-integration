#include <iostream>

#include <gnucash/gnc-engine.h>
#include <gnucash/qofsession.h>

const char* postgresUrl = "postgres://gnc:gnc@127.0.0.1:5432/gnucash";

using std::cout;
using std::endl;

bool qof_error_encountered = false;

void qof_error_check(QofSession *session, const char* func = "")
{
    while(
        qof_session_events_pending(session) ||
        qof_session_save_in_progress(session)
    ) {
        cout << "waiting..." << endl;
        continue;
    }

    QofBackendError error = qof_session_get_error(session);
    if (error != QofBackendError::ERR_BACKEND_NO_ERR)
    {
        cout << "qof session error message: "
            << qof_session_get_error_message(session) << "(" << error << "). "
            << "func: " << func << endl;
        qof_error_encountered = true;
    }
}

class QofSessionRAII {
public:
    QofSession *session;
    QofSessionRAII()
    {
        session = qof_session_new(NULL);
        qof_error_check(session, "qof_session_new");

        qof_session_begin(session, postgresUrl, SessionOpenMode::SESSION_NEW_OVERWRITE);
        qof_error_check(session, "qof_session_begin");
    }
    ~QofSessionRAII()
    {
        qof_session_end(session);
        qof_error_check(session, "qof_session_end");

        qof_session_destroy(session);
        qof_error_check(session, "qof_session_destroy");
    }
};

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

    {
        GncEngineRAII engine;

        QofSessionRAII sessionRaii;
        QofSession * session = sessionRaii.session;
    }

    return qof_error_encountered;
}
