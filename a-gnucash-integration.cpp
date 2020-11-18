#include <iostream>

#include <gnucash/gnc-engine.h>
#include <gnucash/qofsession.h>

static const char* postgresUrl { "postgres://gnc:gnc@127.0.0.1:5432/gnucash" };

using std::cerr;
using std::cout;
using std::endl;
using std::string;

class GncEngineRAII {
public:
    GncEngineRAII()
    {
        cout << "initializing engine..." << endl;
        gnc_engine_init(0, nullptr);
        if (!gnc_engine_is_initialized()) {
            throw string("Engine initialization failed?");
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
    for (auto logModule : { "", "qof", "gnc" })
        qof_log_set_level(logModule, QOF_LOG_DEBUG);

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
    } catch(string s) {
        cerr << "caught string '" << s << "'" << endl;
        return 1;
    }

    return 0;
}
