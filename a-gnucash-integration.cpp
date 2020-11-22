#include <chrono>
#include <iostream>
#include <thread>

#include <gnucash/Account.h>
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
            std::this_thread::sleep_for(std::chrono::seconds(2));
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

    struct CommitError {
        QofSession* session;
        QofBackendError error;
    };

    try {
        GncEngineRAII engine;

        auto book { qof_book_new() };

        auto session { qof_session_new(book) };
        if(qof_session_get_error(session)) throw QSErr { session, "new" };

        gnc_engine_add_commit_error_callback(
            [](void* s, QofBackendError e){
                throw CommitError {static_cast<QofSession*>(s), e }; },
            session);

        qof_session_begin(
            session,
            postgresUrl,
            SessionOpenMode::SESSION_NEW_OVERWRITE
        );
        if(qof_session_get_error(session)) throw QSErr { session, "begin" };

        qof_book_set_backend(book, qof_session_get_backend(session));
        if(qof_session_get_error(session)) throw QSErr { session, "get_backend" };

        auto rootAccount { gnc_book_get_root_account(book) };
    } catch(QSErr err) {
        cerr << string{"qof session error message: "} <<
            qof_session_get_error_message(err.session) << "(" <<
            qof_session_get_error(err.session) <<
            "). from function qof_session_" << err.function << endl;
        return 1;
    } catch(string s) {
        cerr << "caught string '" << s << "'" << endl;
        return 1;
    } catch(CommitError e) {
        cerr << string{"caught CommitError sessionErrMsg("} <<
            qof_session_get_error_message(e.session) << "), QofBackendError="
            << e.error << endl;
        return 1;
    }

    return 0;
}
