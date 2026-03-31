#include "base.hxx"
#include "utils/logger.hxx"

#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h>
#include <vector>

#if BOOST_VERSION >= 109000
    // Boost 1.90+ moved Boost.Process v1 headers into boost/process/v1/
    #include <boost/process/v1/pipe.hpp>
    #include <boost/process/v1/io.hpp>
    #include <boost/process/v1/child.hpp>
    namespace bp = boost::process::v1;
#else
    // Older Boost versions
    #include <boost/process/pipe.hpp>
    #include <boost/process/io.hpp>
    #include <boost/process/child.hpp>
    namespace bp = boost::process;
#endif

#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

using namespace StormByte::VideoConvert;

Task::Execute::Base::Base(const Types::path_t& program, const std::string& arguments)
	: Task::Base(), m_executables({ Executable(program, arguments) }) {}

Task::Execute::Base::Base(Types::path_t&& program, std::string&& arguments)
	: Task::Base(), m_executables({ Executable(std::move(program), std::move(arguments)) }) {}

Task::Execute::Base::Base(const std::vector<Executable>& execs)
	: Task::Base(), m_executables(execs) {}

Task::Execute::Base::Base(std::vector<Executable>&& execs)
	: Task::Base(), m_executables(std::move(execs)) {}

Task::STATUS Task::Execute::Base::do_work(std::optional<pid_t>& worker) noexcept {
	using namespace boost;

	STATUS status = STOPPED;

	if (!m_executables.empty()) {
		asio::io_context ios;

		try {
			std::vector<char> vOut(128 << 10);
			std::vector<char> vErr(128 << 10);
			auto outBuffer{ asio::buffer(vOut) };
			auto errBuffer{ asio::buffer(vErr) };
			auto inBuffer{ asio::buffer(m_stdin) };

			// stdout setup
			bp::pipe pipeOut;
			asio::posix::stream_descriptor sdOut(ios, pipeOut.native_source());

			std::function<void(const system::error_code&, std::size_t)> onStdOut;
			onStdOut = [&](const system::error_code& ec, size_t n) {
				m_stdout.reserve(m_stdout.size() + n);
				m_stdout.insert(m_stdout.end(), vOut.begin(), vOut.begin() + n);
				if (!ec) {
					asio::async_read(sdOut, outBuffer, onStdOut);
				}
			};

			// stderr setup
			bp::pipe pipeErr;
			asio::posix::stream_descriptor sdErr(ios, pipeErr.native_source());

			std::function<void(const system::error_code&, std::size_t)> onStdErr;
			onStdErr = [&](const system::error_code& ec, size_t n) {
				m_stderr.reserve(m_stderr.size() + n);
				m_stderr.insert(m_stderr.end(), vErr.begin(), vErr.begin() + n);
				if (!ec) {
					asio::async_read(sdErr, errBuffer, onStdErr);
				}
			};

			// stdin setup
			bp::pipe pipeIn;
			asio::posix::stream_descriptor sdIn(ios, pipeIn.native_sink());

			if (m_logger)
				m_logger->message_line(Utils::Logger::LEVEL_DEBUG,
					"Executing " + m_executables[0].m_program.string() + " " + m_executables[0].m_arguments);

			bp::child c(
				m_executables[0].m_program.string() + " " + m_executables[0].m_arguments,
				bp::std_out > pipeOut,
				bp::std_err > pipeErr,
				bp::std_in < pipeIn
			);

			asio::async_write(sdIn, inBuffer,
				[&](const system::error_code&, std::size_t) {
					boost::system::error_code ec;
					sdIn.close(ec);
				});

			asio::async_read(sdOut, outBuffer, onStdOut);
			asio::async_read(sdErr, errBuffer, onStdErr);

			worker = c.id();
			ios.run();
			c.wait();
			status = (c.exit_code() == 0) ? HALT_OK : HALT_ERROR;
		}
		catch (const std::exception&) {
			status = HALT_ERROR;
		}
		worker.reset();
	}

	return status;
}

Task::STATUS Task::Execute::Base::pre_run_actions() noexcept {
	m_stdout = "";
	m_stdin = "";
	return RUNNING;
}
