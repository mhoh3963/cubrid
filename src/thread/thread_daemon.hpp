/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

/*
 * thread_daemon - interface for daemon threads
 */

#ifndef _THREAD_DAEMON_HPP_
#define _THREAD_DAEMON_HPP_

#include "thread_looper.hpp"
#include "thread_task.hpp"
#include "thread_waiter.hpp"

#include <thread>

#include <cinttypes>

// cubthread::daemon
//
//  description
//    defines a daemon thread using a looper and a task
//    task is executed in a loop; wait times are defined by looper
//
//  how to use
//    // define your custom task
//    class custom_task : public cubthread::task<custom_thread_context>
//    {
//      void execute (custom_thread_context & context) override { ... }
//    }
//
//    // define your custom context manager
//    class custom_thread_context_manager : public cubthread::context_manager<custom_thread_context>
//    {
//      custom_thread_context & create_context (void) override { ... }
//      void retire_context(custom_thread_context & context) override { ... }
//    }
//
//    // declare a looper
//    cubthread::looper loop_pattern;   // by default sleep until wakeup
//
//    // context manager is required
//    custom_thread_context_manager thr_ctxt_mgr;
//
//    // and finally looping task
//    custom_task *task = new_custom_task ();
//
//    cubthread::daemon my_daemon (loop_pattern, thr_ctxt_mgr, *task);    // daemon starts, executes task and sleeps
//
//    std::chrono::sleep_for (std::chrono::seconds (1));
//    my_daemon.wakeup ();    // daemon executes task again
//    std::chrono::sleep_for (std::chrono::seconds (1));
//
//    // when daemon is destroyed, its execution is stopped and thread is joined
//    // daemon will handle task deletion
//
namespace cubthread
{
  class daemon
  {
    public:

      //  daemon constructor needs:
      //    loop_pattern_arg    : loop pattern for task execution
      //    context_manager_arg : context manager to create and retire thread execution context
      //    exec                : task to execute
      //
      //  NOTE: it is recommended to use dynamic allocation for execution tasks
      //
      template <typename Context>
      daemon (const looper &loop_pattern_arg, context_manager<Context> *context_manager_arg,
	      task<Context> *exec);
      ~daemon();

      void wakeup (void);         // wakeup daemon thread
      void stop_execution (void); // stop_execution daemon thread from looping and join it
      // note: this must not be called concurrently

      bool was_woken_up (void);   // return true if daemon was woken up before timeout

      void reset_looper (void);   // reset looper
      // note: this applies only if looper wait pattern is of type INCREASING_PERIODS

      // statistics
      using stat_type = std::uint64_t;

      // all statistics:
      // own stats: loop count, execute time, pause time = 3
      // + looper stats
      // + waiter stats
      static const std::size_t STAT_COUNT = 3 + looper::STAT_COUNT + waiter::STAT_COUNT;

      void get_stats (stat_type *stats_out);

    private:
      template <typename Context>
      static void loop (daemon *daemon_arg, context_manager<Context> *context_manager_arg,
			task<Context> *exec_arg);     // daemon thread loop function

      void pause (void);                                    // pause between tasks

      waiter m_waiter;        // thread waiter
      looper m_looper;        // thread looper
      std::function<void (void)> m_func_on_stop;  // callback function to interrupt execution on stop request

      std::thread m_thread;   // the actual daemon thread

      // stats
      stat_type m_loop_count;
      stat_type m_execute_time;
      stat_type m_pause_time;

      // todo: m_log
  };

  /************************************************************************/
  /* Inline/template Implementation                                       */
  /************************************************************************/

  template <typename Context>
  daemon::daemon (const looper &loop_pattern_arg, context_manager<Context> *context_manager_arg,
		  task<Context> *exec)
    : m_waiter ()
    , m_looper (loop_pattern_arg)
    , m_func_on_stop ()
    , m_thread ()
    , m_loop_count (0)
    , m_execute_time (0)
    , m_pause_time (0)
  {
    // starts a thread to execute daemon::loop
    m_thread = std::thread (daemon::loop<Context>, this, context_manager_arg, exec);
  }

  template <typename Context>
  void
  daemon::loop (daemon *daemon_arg, context_manager<Context> *context_manager_arg, task<Context> *exec_arg)
  {
    // create execution context
    Context &context = context_manager_arg->create_context ();

    // now that we have access to context we can set the callback function on stop
    daemon_arg->m_func_on_stop = std::bind (&Context::interrupt_execution, std::ref (context));

    // loop until stopped
    using clock_type = std::chrono::high_resolution_clock;

    clock_type::time_point start_timept = clock_type::now ();
    clock_type::time_point end_timept;
    std::chrono::nanoseconds timediff_nano;

    while (!daemon_arg->m_looper.is_stopped ())
      {
	++daemon_arg->m_loop_count;

	// execute task
	exec_arg->execute (context);

	// gather execute stats
	end_timept = clock_type::now ();
	timediff_nano = end_timept - start_timept;
	daemon_arg->m_execute_time += timediff_nano.count ();
	start_timept = end_timept;

	// take a break
	daemon_arg->pause ();

	// gather pause stats
	end_timept = clock_type::now ();
	timediff_nano = end_timept - start_timept;
	daemon_arg->m_pause_time += timediff_nano.count ();
	start_timept = end_timept;
      }

    // retire execution context
    context_manager_arg->retire_context (context);

    // retire task
    exec_arg->retire ();
  }

} // namespace cubthread

#endif // _THREAD_DAEMON_HPP_