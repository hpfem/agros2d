// This file is part of Hermes2D.
//
// Hermes2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D.  If not, see <http://www.gnu.org/licenses/>.

#include "common.h"

using namespace std;

const std::string get_quad_order_str(const int quad_order) {
  std::stringstream str;
  str << "(H:" << get_h_order(quad_order) << ";V:" << get_v_order(quad_order) << ")";
  return str.str();
}

HERMES2D_API void __h2d_exit_if(bool cond, int code) {
  if (cond)
    exit(code);
}

class LoggerMonitor { ///< Logger monitor
  pthread_mutexattr_t mutex_attr; ///< Mutext attributes.
  pthread_mutex_t mutex; ///< Mutex that protects monitor.

public:
  LoggerMonitor() {
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex, &mutex_attr);
  };
  ~LoggerMonitor() {
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&mutex_attr);
  };
  void enter() { pthread_mutex_lock(&mutex); }; ///< enters protected section
  void leave() { pthread_mutex_unlock(&mutex); }; ///< leaves protected section
};
LoggerMonitor logger_monitor; ///< Monitor that protects logging function.

HERMES2D_API bool __h2d_log_message_if(bool cond, const __h2d_log_info& info, const char* msg, ...) {
  if (cond) {
    logger_monitor.enter();

    //print message to a buffer (since vfprintf modifies arglist such that it becomes unusable)
    //not safe, but C does not offer any other multiplatform solution. Since vsnprintf modifies the var-args, it cannot be called repeatedly.
    #define BUF_SZ 2048
    char text[BUF_SZ];
    char* text_contents = text + 1;
    if (msg[0] == ' ')
      text[0] = ' ';
    else {
      text[0] = info.code;
      text[1] = ' ';
      text_contents++;
    }
    va_list arglist;
    va_start(arglist, msg);
    vsprintf(text_contents, msg, arglist);
    va_end(arglist);

    //print location
    ostringstream location;
    //if (info.src_function != NULL)
    //  location << '(' << info.src_function << ')';

    //print the message
    printf("%s %s\n", text, location.str().c_str());

    //print to file
    if (info.log_file != NULL) {
      FILE* file = fopen(info.log_file, "at");
      if (file != NULL)
      {
        //build a long verions of location
        ostringstream location;
        location << '(';
        if (info.src_function != NULL) {
          location << info.src_function;
          if (info.src_file != NULL)
            location << '@';
        }
        if (info.src_file != NULL)
          location << info.src_file << ':' << info.src_line;
        location << ')';

        //get time
        time_t now;
        time(&now);
        struct tm* now_tm = gmtime(&now);
        char time_buf[BUF_SZ];
        strftime(time_buf, BUF_SZ, "%y%m%d-%H:%M", now_tm);

        //write
        fprintf(file, "%s\t%s %s\n", time_buf, text, location.str().c_str());
        fclose(file);
      }
    }

    logger_monitor.leave();
  }
  return cond;
}

void __hermes2d_fwrite(const void* ptr, size_t size, size_t nitems, FILE* stream, const __h2d_log_info& err_info)
{
  if (fwrite(ptr, size, nitems, stream) != nitems || ferror(stream))
    __h2d_exit_if(__h2d_log_message_if(true, err_info, "Error writing to file: %s", strerror(ferror(stream))));
}

void __hermes2d_fread(void* ptr, size_t size, size_t nitems, FILE* stream, const __h2d_log_info& err_info)
{
  size_t ret = fread(ptr, size, nitems, stream);
  if (ret < nitems)
    __h2d_exit_if(__h2d_log_message_if(true, err_info, "Premature end of file."));
  else if (ferror(stream))
    __h2d_exit_if(__h2d_log_message_if(true, err_info, "Error reading file: %s", strerror(ferror(stream))));
}

//// logo //////////////////////////////////////////////////////////////////////////////////
#ifndef HERMED2D_NO_LOGO
class __h2d_logo {
public:
  __h2d_logo() {
    printf("\n-------------------------------------------------\n");
    printf("          This application uses Hermes2D\n");
    printf("       Hermes2D is a C++ library for rapid \n");
    printf(" prototyping of adaptive FEM and hp-FEM solvers\n");
    printf("      developed by the hp-FEM group at UNR\n");
    printf("     and distributed under the GPL license.\n");
    printf("    For more details visit http://hpfem.org/.\n");
    printf("-------------------------------------------------\n");
    printf("The message can be removed by rebulding Hermesd2D\n");
    fflush(stdout);
  }
};
__h2d_logo __h2d_logo_instance;
#endif

//// timing stuff //////////////////////////////////////////////////////////////////////////////////

/*#ifndef WIN32
  #include <sys/time.h>
#else*/
  #include <time.h>
//#endif

static const int max_stack = 50;
static clock_t tick_stack[max_stack];
static int ts_top = 0;

HERMES2D_API void __h2d_begin_time() // TODO: make this return wall time on both Linux and Win32
{
  if (ts_top < max_stack) tick_stack[ts_top] = clock();
  else warn("Timing stack overflow.");
  ts_top++;
}


HERMES2D_API double __h2d_cur_time()
{
  if (!ts_top) { warn("Called without begin_time()."); return -1.0; }
  if (ts_top >= max_stack) return -1.0;
  return (double) (clock() - tick_stack[ts_top-1]) / CLOCKS_PER_SEC;
}


HERMES2D_API double __h2d_end_time()
{
  if (!ts_top) { warn("Called without begin_time()."); return -1.0; }
  ts_top--;
  if (ts_top >= max_stack) return -1.0;
  return (double) (clock() - tick_stack[ts_top]) / CLOCKS_PER_SEC;
}

//// runtime report control varibles //////////////////////////////////////////////////////////////////////////////////
HERMES2D_API bool __h2d_report_warn = true;
HERMES2D_API bool __h2d_report_info = true;
HERMES2D_API bool __h2d_report_verbose = true;
HERMES2D_API bool __h2d_report_trace = true;
HERMES2D_API bool __h2d_report_time = true;
HERMES2D_API bool __h2d_report_debug = true;

//// python support //////////////////////////////////////////////////////////////////////////////////
HERMES2D_API void throw_exception(char *text)
{
  throw std::runtime_error(text);
}

