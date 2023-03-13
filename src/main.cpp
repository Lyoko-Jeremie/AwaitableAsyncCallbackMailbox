#include <iostream>

#include "./MemoryBoost.h"
#include "./OwlLog/OwlLog.h"
#include "./AsyncCallbackMailbox/AsyncCallbackMailbox.h"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/bind/bind.hpp>

using boost::asio::use_awaitable;
#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
  boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

struct B2A;

struct A2B : public boost::enable_shared_from_this<A2B> {

    // Serial2Cmd.runner = Cmd2Serial.callbackRunner
    std::function<void(boost::shared_ptr<B2A>)> callbackRunner;

};

struct B2A : public boost::enable_shared_from_this<B2A> {
    std::function<void(boost::shared_ptr<B2A>)> runner;

};
using ABMailbox =
        boost::shared_ptr<
                OwlAsyncCallbackMailbox::AsyncCallbackMailbox<
                        A2B,
                        B2A
                >
        >;

using MailA2B = ABMailbox::element_type::A2B_t;
using MailB2A = ABMailbox::element_type::B2A_t;

#include <type_traits>

template<
        typename MailboxType /*= boost::shared_ptr<OwlAsyncCallbackMailbox::AsyncCallbackMailbox<MailA2BType, MailB2AType>>*/,
        typename MailA2BType = MailboxType::element_type::A2B_t::element_type,
        typename MailB2AType = MailboxType::element_type::B2A_t::element_type
>
concept GoodMailbox=requires(MailboxType box, boost::shared_ptr<MailA2BType> mail) {
    typename boost::shared_ptr<OwlAsyncCallbackMailbox::AsyncCallbackMailbox<MailA2BType, MailB2AType>>;
    mail->callbackRunner;
    { box->sendA2B(mail->shared_from_this()) };
    { boost::shared_ptr<MailA2BType>{}}->std::same_as<typename MailboxType::element_type::A2B_t>;
    { boost::shared_ptr<MailB2AType>{}}->std::same_as<typename MailboxType::element_type::B2A_t>;
    { boost::shared_ptr<OwlAsyncCallbackMailbox::AsyncCallbackMailbox<MailA2BType, MailB2AType>>{}}->std::same_as<MailboxType>;
    { boost::make_shared<OwlAsyncCallbackMailbox::AsyncCallbackMailbox<MailA2BType, MailB2AType>>() }->std::same_as<MailboxType>;
};


template<
        typename MailboxType,
        typename MailA2BType,
        typename MailB2AType
>
requires GoodMailbox<MailboxType, MailA2BType, MailB2AType>
void fff() {}

// https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/example/cpp20/operations/callback_wrapper.cpp
template<
        typename MailboxType /*= boost::shared_ptr<OwlAsyncCallbackMailbox::AsyncCallbackMailbox<MailA2BType, MailB2AType>>*/,
        typename MailA2BType = MailboxType::element_type::A2B_t::element_type,
        typename MailB2AType = MailboxType::element_type::B2A_t::element_type,
        boost::asio::completion_token_for<void(boost::shared_ptr<MailB2AType>)> CompletionToken
>
requires requires(MailboxType box, boost::shared_ptr<MailA2BType> mail) {
    typename boost::shared_ptr<OwlAsyncCallbackMailbox::AsyncCallbackMailbox<MailA2BType, MailB2AType>>;
    std::same_as<MailboxType, boost::shared_ptr<OwlAsyncCallbackMailbox::AsyncCallbackMailbox<MailA2BType, MailB2AType>>>;
    std::same_as<MailA2BType, typename MailboxType::element_type::A2B_t::element_type>;
    std::same_as<MailB2AType, typename MailboxType::element_type::B2A_t::element_type>;
    mail->callbackRunner;
    { box->sendA2B(mail->shared_from_this()) };
}
auto async_send_mail_2_b(MailboxType box, boost::shared_ptr<MailA2BType> mail, CompletionToken &&token) {
    auto init = [box, mail](
            boost::asio::completion_handler_for<void(boost::shared_ptr<MailB2AType>)> auto handler) mutable {
//        auto work = boost::asio::make_work_guard(handler);

        auto workPtr = boost::make_shared<decltype(boost::asio::make_work_guard(handler))>(
                boost::asio::make_work_guard(handler)
        );
        auto handlerPtr = boost::make_shared<decltype(handler)>(std::move(handler));

        mail->callbackRunner = [
                handlerPtr,
                workPtr,
                mail
        ](boost::shared_ptr<MailB2AType> m) {

            auto alloc = boost::asio::get_associated_allocator(*handlerPtr, boost::asio::recycling_allocator<void>());

            boost::asio::dispatch(
                    workPtr->get_executor(),
                    boost::asio::bind_allocator(
                            alloc,
                            [
                                    handlerPtr,
                                    m
                            ]() {
                                (*handlerPtr)(m);
                            }));
        };
        box->sendA2B(mail->shared_from_this());
    };

    return boost::asio::async_initiate<CompletionToken, void(boost::shared_ptr<MailB2AType>)>(
            init,
            token);
}

boost::asio::awaitable<bool> a(ABMailbox box) {
    auto executor = co_await boost::asio::this_coro::executor;

    try {

        for (;;) {

            co_await boost::asio::post(executor, use_awaitable);
            {
                auto m = boost::make_shared<A2B>();

                fff<ABMailbox, A2B, B2A>();

                auto rB2A = co_await async_send_mail_2_b(
                        box->shared_from_this(), m,
                        boost::asio::bind_executor(
                                executor,
                                use_awaitable));
                rB2A->runner;

            }

            co_return false;
        }

    } catch (const std::exception &e) {

    }

    co_return true;
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    auto ioc = boost::asio::io_context{};

    auto mailbox = boost::make_shared<ABMailbox::element_type>(
            ioc, ioc, "ABMailbox"
    );

    boost::asio::co_spawn(ioc, [mailbox] {
        return a(mailbox);
    }, [](std::exception_ptr e, bool n) {

        // https://stackoverflow.com/questions/14232814/how-do-i-make-a-call-to-what-on-stdexception-ptr
        try { std::rethrow_exception(std::move(e)); }
        catch (const std::exception &e) {
            BOOST_LOG_OWL(error) << "co_spawn catch std::exception " << e.what();
        }
        catch (const std::string &e) { BOOST_LOG_OWL(error) << "co_spawn catch std::string " << e; }
        catch (const char *e) { BOOST_LOG_OWL(error) << "co_spawn catch char *e " << e; }
        catch (...) {
            BOOST_LOG_OWL(error) << "co_spawn catch (...)"
                                 << "\n" << boost::current_exception_diagnostic_information();
        }
    });

    ioc.run();


    return 0;
}
