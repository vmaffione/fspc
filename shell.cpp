/*
 *  fspc shell support
 *
 *  Copyright (C) 2013-2014  Vincenzo Maffione
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <string>
#include <list>
#include <sstream>
#include <cstdio>
#include <cstring>       /* strlen() */
#include <unistd.h>      /* fork() */
#include <sys/wait.h>    /* waitpid() */
#include <ncurses.h>
#include <fcntl.h>

#include "shell.hpp"

/* FspDriver class to access the compiler structure. */
#include "fsp_driver.hpp"

/* Lts definitions and operations. */
#include "lts.hpp"

/* Helper functions. */
#include "helpers.hpp"

/* ShDriver class to access expression parser. */
#include "sh_driver.hpp"


#define DEBUG
#ifdef DEBUG
#define IFD(x)  (x)
#else
#define IFD(x)
#endif


#define HISTORY_MAX_COMMANDS    20

/* =========================== Shell implementation ==================== */
void Shell::common_init()
{
    /*
     * Initialize the help map.
     */
    help_map["ls"] = HelpEntry("ls", "Show a list of compiled FSPs");
    help_map["safety"] = HelpEntry("safety [FSP_NAME]",
                            "Run deadlock/error analysis on "
                            "the specified FSP or on every FSP");
    help_map["progress"] = HelpEntry("progress [FSP_NAME]",
                            "Run progress analysis on "
                            "the specified FSP or on every FSP");
    help_map["simulate"] = HelpEntry("simulate FSP_NAME",
                            "Run an interactive simulation of "
                            "the specified FSP");
    help_map["basic"] = HelpEntry("basic FSP_NAME FILE_NAME",
                            "Write a basic process description of the "
                            "specified FSP into the specified output file");
    /* help_map["monitor"] = HelpEntry("monitor "
                            "FSP_NAME INTERACTIONS [-o FILE]",
                            "Write, if possible, the Monitor Normal Form "
                            "of the specified FSP into the specified "
                            "output file"); */
    /* help_map["code"] = HelpEntry("UNKNOWN", "UNKNOWN"); */
    help_map["alpha"] = HelpEntry("alpha FSP_NAME",
                                "Show the alphabet of the specified FSP");
    help_map["see"] = HelpEntry("see FSP_NAME",
                            "Show a graphical representation of the "
                            "specified FSP using GraphViz");
    help_map["print"] = HelpEntry("print FSP_NAME {png | pdf}",
                        "Print the GraphViz representation of the specified "
                        "fsp into a file FSP_NAME.FORMAT");
    help_map["lsprop"] = HelpEntry("lsprop",
                            "Show a list of compiled properties");
    help_map["lsmenu"] = HelpEntry("lsmenu",
                            "Show a list of available menus");
    help_map["minimize"] = HelpEntry("minimize FSP_NAME", "Minimize the "
                              "specified FSP");
    /* help_map["traces"] = HelpEntry("traces FSP_NAME",
                            "Find all the action traces for the specified "
                            "process, stopping when there are cycles"); */
    help_map["printvar"] = HelpEntry("printvar [VAR_NAME]",
                            "Print the value of the specified variable or "
                            "of all the variables VAR_NAME");
    help_map["if"] = HelpEntry("if CONDITION", "If the specified condition "
                        "is true, the following commands will be executed "
                        "(up to the next matching 'elif', 'else' or 'fi' "
                        "command)");
    help_map["elif"] = HelpEntry("elif CONDITION", "If the specified "
                        "condition is true and all the conditions "
                        "associated to the previous branches were false, "
                        "the following commands will be executed (up to the "
                        "next matching 'elif', 'else' or 'fi' command)");
    help_map["else"] = HelpEntry("else", "If all the conditions associated "
                        "to the previous branches were false, the following "
                        "commands will be executed (up to the next matching "
                        "'else' command)");
    help_map["fi"] = HelpEntry("fi", "Close the last opened 'if', 'elif' or "
                        "'else' branch");
    help_map["graphviz"] = HelpEntry("graphviz FSP_NAME [FILENAME]",
                                     "Output a GraphViz representation "
                                     "of the specified FSP into the "
                                     "specified file (default name is "
                                     "'FSP_NAME.gv')");
    help_map["option"] = HelpEntry("option [OPTION_NAME] [OPTION_VALUE]",
                                   "If no arguments are provided, show all "
                                   "the current shell options. If only the "
                                   "first argument is provided, show the "
                                   "value of the specified option. Otherwise "
                                   "the specified option is set to the value "
                                   "specified by the second argument.");
    help_map["help"] = HelpEntry("help",  "Show this help");
    help_map["exit"] = HelpEntry("exit [EXPRESSION]",
                            "Exit the shell with the specified return code "
                            "(default 0)");
    help_map["quit"] = HelpEntry("quit", "Force the shell to terminate");

    /*
     * Initialize the command map.
     */
    cmd_map["ls"] = &Shell::ls;
    cmd_map["safety"] = &Shell::safety;
    cmd_map["progress"] = &Shell::progress;
    cmd_map["simulate"] = &Shell::simulate;
    cmd_map["basic"] = &Shell::basic;
    /* cmd_map["monitor"] = &Shell::monitor; */
    /* cmd_map["code"] = &Shell::code; */
    cmd_map["alpha"] = &Shell::alpha;
    cmd_map["see"] = &Shell::see;
    cmd_map["print"] = &Shell::print;
    cmd_map["lsprop"] = &Shell::lsprop;
    cmd_map["lsmenu"] = &Shell::lsmenu;
    cmd_map["minimize"] = &Shell::minimize;
    /* cmd_map["traces"] = &Shell::traces; */
    cmd_map["printvar"] = &Shell::printvar;
    cmd_map["if"] = &Shell::if_;
    cmd_map["elif"] = &Shell::elif_;
    cmd_map["else"] = &Shell::else_;
    cmd_map["fi"] = &Shell::fi_;
    cmd_map["graphviz"] = &Shell::graphviz;
    cmd_map["option"] = &Shell::option;
    cmd_map["exit"] = &Shell::exit_;
    cmd_map["help"] = &Shell::help;

    /*
     * Initialize the options map.
     */
    options["label-compression"] = ShellOption("label-compression", "y",
                                               ShellOption::Boolean);

    ifframes.push(IfFrame(true, false, false));
}

/* This function must be called after common_init(). */
void Shell::fill_completion()
{
    map<string, fsp::Symbol *>::iterator it;
    map<string, HelpEntry>::iterator jt;

    /* Parameteric process names. */
    for (it=c.parametric_processes.table.begin();
            it!=c.parametric_processes.table.end(); it++) {
        completion.insert(it->first);
    }

    /* Process names. */
    for (it=c.processes.table.begin(); it!=c.processes.table.end(); it++) {
        completion.insert(it->first);
    }

    trace_processes_size = c.processes.size();

    /* fspc shell command names. */
    for (jt = help_map.begin(); jt != help_map.end(); jt++) {
        completion.insert(jt->first);
    }
}

Shell::Shell(FspDriver& cr, istream& inr) : c(cr), in(inr)
{
    common_init();
    interactive = true;
    history_enabled = true;

    /* Autocompletion initialization. */
    fill_completion();

    /* Curses initialization. */
    initscr();
    cbreak(); //raw();
    noecho();
    keypad(stdscr, TRUE);

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);

    //scrollok(stdscr, TRUE); /* Use original terminal scrolling. */
}

Shell::Shell(FspDriver& cr, ifstream& inr) : c(cr), in(inr)
{
    common_init();
    interactive = false;
}

Shell::~Shell()
{
    if (interactive)
  endwin();   /* Exit curses mode. */
}

static void scroll_screen(int n, int y, int x)
{
    int rows, cols;
    int r_i, c_i;
    chtype ch;

    getmaxyx(stdscr, rows, cols);
    if (n >= rows)
  return;

    /* Shift everything n rows up. */
    for (r_i=n; r_i<rows; r_i++) {
  for (c_i=0; c_i<cols; c_i++) {
      ch = mvinch(r_i, c_i);
      mvaddch(r_i-n, c_i, ch);
  }
    }

    /* Clear the last n rows. */
    move(rows-n, 0);
    clrtobot();

    /* Move the cursor to the required position. */
    move(y, x);
    refresh();
}

/* eol: if false a newline will not be printed together with the last
  line in 'ss'. */
void Shell::putsstream(stringstream& ss, bool eol) {

    if (interactive) {
  string line;
  int y, x;
  int rows, cols;
  bool first_line = true;

  getmaxyx(stdscr, rows, cols);

  /* Split the stringstream content in lines (using '\n' as
     separator. */
  while (getline(ss, line)) {
      int rows_required;

      if (!first_line) {
    /* Newline attached to the line printed in the previous
       iteration: We need to defer the newline insertion because
       of the 'eol' parameter. */
    printw("\n");
    refresh();
      }
      first_line = false;

      /* Compute the number of rows that are necessary to print the
         line. */
      rows_required = line.size() / cols;
      if (line.size() % cols)
    rows_required++;

      getyx(stdscr, y, x);
      /* If there is not enough empty rows in the screen, we scroll as
         many times as needed to make enough room, and move the
         cursor at the beginning of the first empty row after
         scrolling. */
      if (y + rows_required >= rows)
    scroll_screen(y + rows_required - rows + 1,
        rows - rows_required-1, 0);
      /* Finally print the line. */
      printw("%s", line.c_str());
  }

  /* Put a newline after the last line in 'ss' only if asked by
     the user. */
  if (eol)
      printw("\n");

  refresh();
    }
    else {
  cout << ss.str();
    }
}

/* The more portable way I found out to implement 'isprint'. */
static bool is_printable(int ch)
{
    return ch>=32 && ch<=126;
}

static void right_split(string& base, string& back)
{
    size_t p;

    back.clear();

    p = base.find_last_of(' ');
    if (p == string::npos) {
        base.swap(back);
        return;
    }

    back = base.substr(p + 1);
    base = base.substr(0, p + 1);
}

void Shell::getline_ncurses(string& line, const char *prompt)
{
    int ch;

    /* Position of the cursor. */
    int y, x;

    /* Position of the prompt. */
    int prompt_y, prompt_x;

    /* Position of the last char of the command string. */
    int frontier_y, frontier_x;

    /* Command string index of the cursor. */
    int str_cursor = 0;

    /* Number of rows and columns in the screen. */
    int rows, cols;

    int tmp_y, tmp_x;
    string arg;

    getmaxyx(stdscr, rows, cols);

    /* Print the prompty (if any), and initialize the prompt position
       properly. */
    if (prompt) {
  attron(COLOR_PAIR(2));
  printw(prompt);
  attroff(COLOR_PAIR(2));
  refresh();
    }
    getyx(stdscr, prompt_y, prompt_x);

    /* Initially the frontier is the same as the prompt. */
    frontier_y = prompt_y;
    frontier_x = prompt_x;

    /* Always start with an empty string. */
    line = string();

    for (;;) {
  ch = getch();
  getyx(stdscr, y, x);

  switch (ch) {
      case '\r':
      case '\n':
    move(frontier_y, frontier_x);
    if (frontier_y == rows-1) {
        scroll_screen(1, frontier_y-1,frontier_x);
    }
    printw("\n");
    refresh();
                if (history_enabled) {
                    /* Update the history. */
                    history.add_command(line);
                }
    return;

      case KEY_UP:
      case KEY_DOWN:
                if (!history_enabled) {
                    break;
                }
                if (ch == KEY_UP) {
                    history.up();
                } else /* (ch == KEY_DOWN) */ {
                    history.down();
                }
                history.get_current(line);
                move(prompt_y, prompt_x);
                printw("%s", line.c_str());
    getyx(stdscr, frontier_y, frontier_x);
                move(frontier_y, frontier_x);
                str_cursor = line.size();
    clrtobot();
    break;

            case '\t':
                /* Split the current 'line' into 'line' + 'arg', so that
                   'arg' contains the last command word. */
                right_split(line, arg);
                /* Try to complete 'arg'. */
                if (completion.lookup(arg)) {
                    /* The string 'arg' has been extended because of
                       auto-completion. Reflush the command string. */
                    line += arg;
                    move(prompt_y, prompt_x);
                    printw("%s", line.c_str());
                    getyx(stdscr, frontier_y, frontier_x);
                    move(frontier_y, frontier_x);
                    str_cursor = line.size();
                    clrtobot();
                } else {
                    /* The string 'arg' was not modified: Let's undo the
                       splitting. */
                    line += arg;
                }
                break;

      case KEY_LEFT:
    if (y > prompt_y || x > prompt_x) {
        if (x) {
      x--;
        } else {
      y--;
      x = cols-1;
        }
        move(y, x);
        str_cursor--;
    }
    break;

      case KEY_RIGHT:
    if (y < frontier_y || x < frontier_x) {
        if (x == cols-1) {
      x = 0;
      y++;
        } else {
      x++;
        }
        move(y, x);
        str_cursor++;
    }
    break;

      case KEY_HOME:
    move(prompt_y, prompt_x);
    str_cursor = 0;
    break;

      case KEY_END:
    move(frontier_y, frontier_x);
    str_cursor = line.size();
    break;

      case 127: /* Backspace. */
      case KEY_BACKSPACE:
    if (str_cursor) {
        /* Compute the next cursor position. */
        if (x) {
      x--;
        } else {
      y--;
      x = cols-1;
        }
        str_cursor--;
        /* Use the same implementation of KEY_DC. */
    } else break;

      case KEY_DC:    /* Canc */
    if (line.size()) {
        line.erase(str_cursor, 1);

        /* Update the frontier. */
        if (frontier_x) {
      frontier_x--;
        } else {
      frontier_x = cols-1;
      frontier_y--;
        }

        /* Reflush the command string. */
        move(prompt_y, prompt_x);
        printw("%s", line.c_str());

        /* Clear up to the end of the screen, in order to remove
           old trailing character (they have been shifted). */
        clrtobot();

        /* Restore the cursor position. */
        move(y, x);
    }
    break;

      default:
    if (is_printable(ch)) {
        /* Insert a character in the command string at the
           current cursor position. */
        line.insert(str_cursor, 1, static_cast<char>(ch));

        /* Compute the new cursor position. */
        if (x == cols-1) {
      x = 0;
      if (y == rows-1) {
          prompt_y--;
          scroll_screen(1, rows - 1, 0);
          frontier_y = rows - 2;
      } else {
          y++;
      }
        } else {
      x++;
        }
        str_cursor++;

        /* Reflush the whole command string. */
        move(prompt_y, prompt_x);
        printw("%s", line.c_str());

        /* Update the frontier. */
        getyx(stdscr, tmp_y, tmp_x);
        if (tmp_y > frontier_y) {
      frontier_y = tmp_y;
      frontier_x = 0;
        }
        if (tmp_y == frontier_y)
      frontier_x = max(frontier_x, tmp_x);

        /* Restore the cursor position. */
        move(y, x);
    }
  }
  refresh();
    }

    assert(rows < 10000);
}

void Shell::readline(string& line)
{
    if (interactive) {
  getline_ncurses(line, NULL);
    } else {
        getline(in, line);
    }
}

int Shell::ls(const vector<string> &args, stringstream& ss)
{
    map<string, fsp::Symbol *>::iterator it;
    fsp::Lts *lts;

    ss << "Available FSPs:\n";
    /* Compiled processes (e.g. all the processes in the 'processes'
       cache table). */
    for (it = c.processes.table.begin();
                it != c.processes.table.end(); it++) {
  lts = fsp::is<fsp::Lts>(it->second);
  ss << "   " << it->first << ": " << lts->numStates()
      << " states, " << lts->numTransitions() << " transitions, "
      << lts->alphabetSize() << " actions in alphabet\n";
    }
    /* Processes defined in the FSP input file that have not been
       translated yet. */
    for (it = c.parametric_processes.table.begin();
                it != c.parametric_processes.table.end(); it++) {
        /* We try to see if the parametric process 'it->first', with
           default parameters, is in the 'processes' cache. If not, we
           don't force the translation. */
        if (c.getLts(it->first, false) == NULL) {
            ss << "   " << it->first << ": NOT TRANSLATED\n";
        }
    }

    return 0;
}

int Shell::safety(const vector<string> &args, stringstream& ss)
{
    map<string, fsp::Symbol *>::iterator it;
    int deadlocks = 0;

    if (args.size()) {
        fsp::SmartPtr<fsp::Lts> lts;

  /* Deadlock analysis on args[0]. */
        lts = c.getLts(args[0], true);
        if (lts == NULL) {
        ss << "Process " << args[0] << " not found\n";
            return -1;
        }
        deadlocks = lts->deadlockAnalysis(ss);
    } else {
        fsp::Lts *lts;

  /* Deadlock analysis on every process. */
  for (it=c.processes.table.begin();
        it!=c.processes.table.end(); it++) {
      lts = fsp::is<fsp::Lts>(it->second);
      deadlocks += lts->deadlockAnalysis(ss);
  }
    }

    return deadlocks;
}

int Shell::progress(const vector<string> &args, stringstream& ss)
{
    map<string, fsp::Symbol *>::iterator it;
    map<string, fsp::Symbol *>::iterator jt;
    fsp::ProgressS *pv;
    int npv = 0;    /* Number of progress violations. */

    if (args.size()) {
        fsp::SmartPtr<fsp::Lts> lts;

  /* Progress analysis on args[0]. */
        lts = c.getLts(args[0], true);
        if (lts == NULL) {
        ss << "Process " << args[0] << " not found\n";
            return -1;
        }
        for (it=c.progresses.table.begin();
                it!=c.progresses.table.end(); it++) {
            pv = fsp::is<fsp::ProgressS>(it->second);
            npv = lts->progress(it->first, *pv, ss);
        }
    } else {
        fsp::Lts *lts;

  /* Progress analysis on every process. */
  for (it=c.processes.table.begin();
        it!=c.processes.table.end(); it++) {
      lts = fsp::is<fsp::Lts>(it->second);
      for (jt=c.progresses.table.begin();
        jt!=c.progresses.table.end(); jt++) {
    pv = fsp::is<fsp::ProgressS>(jt->second);
    npv += lts->progress(jt->first, *pv, ss);
      }
  }
    }

    return npv;
}

int Shell::simulate(const vector<string> &args, stringstream& ss)
{
    fsp::SmartPtr<fsp::Lts> lts;
    fsp::ActionSetS *menu = NULL;

    if (!args.size()) {
    ss << "Invalid command: try 'help'\n";
        return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
        ss << "Process " << args[0] << " not found\n";
        return -1;
    }

    if (args.size() >= 2) {
        fsp::Symbol *svp;

        if (!c.menus.lookup(args[1], svp)) {
            ss << "Menu " << args[1] << " not found\n";
            return -1;
        }
        menu = fsp::is<fsp::ActionSetS>(svp);
    }

    history_enable(false);
    lts->simulate(*this, menu);
    history_enable(true);

    return 0;
}

int Shell::basic(const vector<string> &args, stringstream& ss)
{
    string outfile;
    fsp::SmartPtr<fsp::Lts> lts;

    if (!args.size()) {
    ss << "Invalid command: try 'help'\n";
    return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
        ss << "Process " << args[0] << " not found\n";
        return -1;
    }

    if (args.size() >= 2) {
  outfile = args[1];
    } else {
  outfile = args[0] + ".bfsp";
    }

    lts->basic(outfile, ss);

    return 0;
}

int Shell::monitor(const vector<string>& args, stringstream& ss)
{
    string outfile(args[0] + ".mfsp");
    fsp::SmartPtr<fsp::Lts> lts;

    if (!args.size()) {
        ss << "Invalid command: try 'help'\n";
        return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
        ss << "Process " << args[0] << " not found\n";
        return -1;
    }

    std::list<std::string> interactions;
    bool outputFlagSet = false;
    bool fileNameRead = false;
    std::string outputFlag("-o");

    if (args.size() >= 2) {
        /* outfile = args[1]; */
        for (unsigned int i = 1; i < args.size(); i++) {
            if (outputFlag.compare(args[i]) == 0 && (!outputFlagSet)) {
                outputFlagSet = true;
                continue;
            } else if (outputFlagSet && (!fileNameRead)) {
                outfile = string(args[i]);
                fileNameRead = true;
                continue;
            }
            interactions.push_back(std::string(args[i]));
        }
    }

    string representation;

    if (!coder.get_monitor_representation(*lts, interactions,
                                                representation)) {
        ss << representation;
        return -1;
    } else {
        fstream file;
        file.open(outfile.c_str(), ios::out);
        if (!file.is_open() || !(file << representation)) {
            ss << "Cannot open file " << outfile;
        }
        file.close();
    }
    return 0;
}

int Shell::code(const vector<string>& args, stringstream& ss)
{
    string outfile(args[0] + ".java");
    fsp::SmartPtr<fsp::Lts> lts;

    if (!args.size()) {
        ss << "Invalid command: try 'help'\n";
        return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
        ss << "Process " << args[0] << " not found\n";
        return -1;
    }

    std::list<std::string> interactions;
    bool outputFlagSet = false;
    bool fileNameRead = false;
    std::string outputFlag("-o");

    if (args.size() >= 2) {
        /* outfile = args[1]; */
        for (unsigned int i = 1; i < args.size(); i++) {
            if (outputFlag.compare(args[i]) == 0 && (!outputFlagSet)) {
                outputFlagSet = true;
                continue;
            } else if (outputFlagSet && (!fileNameRead)) {
                outfile = string(args[i]);
                fileNameRead = true;
                continue;
            }
            interactions.push_back(std::string(args[i]));
        }
    }

    string code;

    if (!coder.instantiate_monitor_template(*lts, interactions, args[0], code)) {
        ss << code;
        return -1;
    } else {
        fstream file;
        file.open(outfile.c_str(), ios::out);
        if (!file.is_open() || !(file << code)) {
            ss << "Cannot open file " << outfile;
        }
        file.close();
    }
    return 0;
}

int Shell::alpha(const vector<string> &args, stringstream& ss)
{
    fsp::SmartPtr<fsp::Lts> lts;

    if (!args.size()) {
    ss << "Invalid command: try 'help'\n";
    return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
    ss << "Process " << args[0] << " not found\n";
    return -1;
    }

    lts->printAlphabet(ss, options["label-compression"].get() == "y");

    return 0;
}

#define SEE_MAX_STATES  100

int Shell::see(const vector<string> &args, stringstream& ss)
{
    fsp::SmartPtr<fsp::Lts> lts;
    string tmp_name, stdout_tmp_name;
    pid_t drawer;
    const char *exec_errmsg;
    int ret;

    if (!interactive) {
    ss << "Cannot use 'see' command in scripts\n";
    return -1;
    }

    if (!args.size()) {
    ss << "Invalid command: try 'help'\n";
    return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
    ss << "Process " << args[0] << " not found\n";
    return -1;
    }

    if (lts->numStates() == 0) {
        ss << "Lts " << args[0] << " is empty\n";
        return 0;
    }

    if (lts->numStates() > SEE_MAX_STATES) {
        ss << "Lts " << args[0] << " has too many states\n";
        return 0;
    }

    /* Generate the graphivz output into a temporary file (whose name does
       not collide with other fspc instances). */
    tmp_name = get_tmp_name("", "gv");
    lts->graphvizOutput(tmp_name.c_str(), options["label-compression"].get() == "y");

    /* UNIX-specific section. */
    stdout_tmp_name = get_tmp_name("", "stdout.tmp");
    drawer = fork();

    switch (drawer) {
    case -1:
        ss << "fork() error\n";
        return -1;
        break;
    case 0:
        /* This is executed by the child. */

        /* A Unix trick used to redirect the standard output to a file.
           Just close the standard output and immediately open the file to
           redirect into: Unix semantic guarantees that the file descriptor
           for the new file will be the lowest unused, and so in this case
           stdout will be selected. */
        close(1);
        open(stdout_tmp_name.c_str(), O_CREAT | O_WRONLY, S_IRUSR);

        execl("ltsee", "ltsee", tmp_name.c_str(), NULL);
        execlp("ltsee", "ltsee", tmp_name.c_str(), NULL);
        perror("ltsee exec failed");
        exec_errmsg = "Cannot find ltsee";
        ret = write(1, exec_errmsg, strlen(exec_errmsg) + 1);
        (void)ret;
        exit(EXIT_FAILURE);
        break;
    default:
        /* This is executed by the parent. */
        int status;

        waitpid(-1, &status, 0);

        /* We read from the standard output of the child an write what
           we read to the console output. It is important to open the
           file after waitpid(), otherwise there is a race condition
           where we try to open the file before this is created by the
           child. */
        ifstream stdout_file(stdout_tmp_name.c_str());

        if (stdout_file) {
            ss << stdout_file.rdbuf() << "\n";
        } else {
            ss << "Error: Make sure you have write permissions in the "
                    "current directory\n";
        }

        remove(stdout_tmp_name.c_str());
    }

    remove(tmp_name.c_str());

    return 0;
}

int Shell::print(const vector<string> &args, stringstream& ss)
{
    fsp::SmartPtr<fsp::Lts> lts;
    string format = "png";
    string filename, stdout_tmp_name;
    const char *exec_errmsg = NULL;
    int ret;

    if (!args.size()) {
    ss << "Invalid command: try 'help'\n";
    return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
    ss << "Process " << args[0] << " not found\n";
    return -1;
    }
    filename = args[0] + ".gv";

    lts->graphvizOutput(filename.c_str(), options["label-compression"].get() == "y");

    if (args.size() > 1) {
        if (args[1] != "png" && args[1] != "pdf") {
            ss << "Format '" << args[1] << "' unknown\n";
            return -1;
        }
        format = args[1];
    }

    /* UNIX-specific section. */
    stdout_tmp_name = get_tmp_name("", "stdout.tmp");
    pid_t drawer = fork();

    switch (drawer) {
    case -1:
        ss << "fork() error\n";
        return -1;
        break;
    case 0:
        close(1);
        open(stdout_tmp_name.c_str(), O_CREAT | O_WRONLY, S_IRUSR);
        execl("ltsimg", "ltsimg", filename.c_str(), format.c_str(), NULL);
        execlp("ltsimg", "ltsimg", filename.c_str(), format.c_str(), NULL);
        exec_errmsg = "Cannot find ltsimg";
        ret = write(1, exec_errmsg, strlen(exec_errmsg) + 1);
        (void)ret;
        exit(EXIT_FAILURE);
        break;
    default:
        int status;

        waitpid(-1, &status, 0);

        ifstream stdout_file(stdout_tmp_name.c_str());

        if (stdout_file) {
            ss << stdout_file.rdbuf() << "\n";
        } else {
            ss << "Error: Make sure you have write permissions in the "
                    "current directory\n";
        }

        remove(stdout_tmp_name.c_str());
    }

    remove(filename.c_str());

    return 0;
}

int Shell::lsprop(const vector<string> &args, stringstream& ss)
{
    map<string, fsp::Symbol *>::iterator it;
    fsp::ProgressS *pv;

    ss << "Progresses:\n";
    for (it=c.progresses.table.begin(); it!=c.progresses.table.end(); it++) {
        fsp::SetS set;
        fsp::SetS cond;

        pv = fsp::is<fsp::ProgressS>(it->second);
        pv->set.toSetValue(set);
        if (pv->conditional) {
            pv->condition.toSetValue(cond);
        }
  ss << "   " << it->first << ": ";
        if (pv->conditional) {
            ss << "if ";
            cond.output(ss);
            ss << " then ";
        }
        set.output(ss);
        ss << "\n";
    }

    return 0;
}

int Shell::lsmenu(const vector<string> &args, stringstream& ss)
{
    map<string, fsp::Symbol *>::iterator it;
    fsp::ActionSetS *as;

    ss << "Menus:\n";
    for (it=c.menus.table.begin(); it!=c.menus.table.end(); it++) {
        fsp::SetS setv;

        as = fsp::is<fsp::ActionSetS>(it->second);
        as->toSetValue(setv);
  ss << "   " << it->first << ": ";
        setv.output(ss);
        ss << "\n";
    }

    return 0;
}

int Shell::minimize(const vector<string> &args, stringstream& ss)
{
    fsp::SmartPtr<fsp::Lts> lts;

    if (!args.size()) {
    ss << "Invalid command: try 'help'\n";
    return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
    ss << "Process " << args[0] << " not found\n";
    return -1;
    }

    lts->minimize(ss);

    return 0;
}

int Shell::traces(const vector<string> &args, stringstream& ss)
{
    fsp::SmartPtr<fsp::Lts> lts;

    if (!args.size()) {
    ss << "Invalid command: try 'help'\n";
    return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
    ss << "Process " << args[0] << " not found\n";
    return -1;
    }

    lts->traces(ss);

    return 0;
}

int Shell::printvar(const vector<string> &args, stringstream& ss)
{
    if (args.size()) {
        if (!variables.count(args[0])) {
            ss << "Variable " << args[0] << " undefined\n";
            return -1;
        }
        ss << "    " << args[0] << " = " << variables[args[0]] << "\n";

        return 0;
    }

    map<string, int>::iterator mit;
    ss << "Defined variables:\n";
    for (mit = variables.begin(); mit != variables.end(); mit++) {
        ss << "    " << mit->first << " = " << mit->second << "\n";
    }

    return 0;
}

int Shell::if_(const vector<string> &args, stringstream& ss)
{
    if (!ifframes.top().accepting) {
        /* We met an 'if' command while not accepting commands. It's
           not necessary to evaluate the condition, because no command
           in the new IfFrame will ever be executed. Therefore we just
           push a new IfFrame, where we don't initially accept, but
           pretend we have already accepted commands.
        */
        ifframes.push(IfFrame(false, true, false));
    } else {
        /* We met an 'if' command while accepting commands. We evaluate
           the condition to initialize the nested IfFrame. */
        ShDriver shd(*this);
        string expression;
        int ret;

        merge_string_vec(args, expression, " ");

        ret = shd.parse(expression);
        if (ret) {
            ss << "    invalid expression '" << expression << "'\n";

            return -1;
        }

        /* Push a new frame, where 'else' has not met yet. The
           nested commands are immediately accepted or not, depending
           on the 'if' condition being true or false. */
        ifframes.push(IfFrame(!!shd.result, !!shd.result, false));
    }

    return 0;
}

int Shell::elif_(const vector<string> &args, stringstream& ss)
{
    if (ifframes.size() == 1 || ifframes.top().else_met) {
        /* We haven't met an 'if' commands that match this 'elif' branch,
           or we have already met an 'else'. This is a semantic error. */
        ss << "    Error: unmatched 'elif'\n";

        return -1;
    }

    if (ifframes.top().accepted) {
        /* We have already accepted commands, so we cannot accept commands
           in this branch, irrespective of the condition. */
        ifframes.top().accepting = false;
    } else {
        /* Evaluate the condition to see whether we should start
           accepting commands in this branch.  */
        ShDriver shd(*this);
        string expression;
        int ret;

        merge_string_vec(args, expression, " ");

        ret = shd.parse(expression);
        if (ret) {
            ss << "    invalid expression\n";

            return -1;
        }

        ifframes.top().accepted = !!shd.result;
        ifframes.top().accepting = !!shd.result;
    }

    return 0;
}

int Shell::else_(const vector<string> &args, stringstream& ss)
{
    if (args.size()) {
	ss << "This command takes no arguments\n";

	return -1;
    }

    if (ifframes.size() == 1 || ifframes.top().else_met) {
        /* We haven't met an 'if' commands that match this 'else' branch,
           or we have already met an 'else'. This is a semantic error. */
        ss << "    Error: unmatched 'else'\n";

        return -1;
    }

    /* Start accepting in this 'else' branch if we haven't already accepted
       in the past. */
    ifframes.top().accepting = !ifframes.top().accepted;
    ifframes.top().accepted = true;
    ifframes.top().else_met = true;

    return 0;
}

int Shell::fi_(const vector<string> &args, stringstream& ss)
{
    if (args.size()) {
	ss << "This command takes no arguments\n";

	return -1;
    }

    if (ifframes.size() == 1) {
        /* No 'if' matches this 'fi' statement: Semantic error. */
        ss << "    Error: unmatched 'fi'\n";

        return -1;
    }

    /* Pop the current IfFrame, returning to the previous one. */
    ifframes.pop();

    return 0;
}

int Shell::graphviz(const vector<string> &args, stringstream& ss)
{
    string outfile;
    fsp::SmartPtr<fsp::Lts> lts;

    if (!args.size()) {
        ss << "Invalid command: try 'help'\n";
        return -1;
    }

    lts = c.getLts(args[0], true);
    if (lts == NULL) {
        ss << "Process " << args[0] << " not found\n";
        return -1;
    }

    if (args.size() >= 2) {
        outfile = args[1];
    } else {
        outfile = args[0] + ".gv";
    }

    lts->graphvizOutput(outfile.c_str(), options["label-compression"].get() == "y");

    return 0;
}

int Shell::option(const vector<string>& args, stringstream& ss)
{
    if (args.size() == 0) {
        /* List all options names and values. */
        for (map<string, ShellOption>::iterator mit = options.begin();
                                            mit != options.end(); mit++) {
            ss << "    " << mit->first << ": '" << mit->second.get() << "'\n";
        }
    } else {
        if (!options.count(args[0])) {
            ss << "    Unrecognized option " << args[0] << "\n";
            return -1;
        }
        if (args.size() == 1) {
            /* Show the value of the option specified with the first
               argument. */
            ss << "    " << args[0] << ": '" <<
                    options[args[0]].get() << "'\n";
        } else {
            /* Set the value of the option specified with the first
               argument. */
            int ret;

            ret = options[args[0]].set(args[1]);

            if (ret) {
                ss << "    Invalid option value\n";
            }

            return ret;
        }
    }

    return 0;
}

int Shell::exit_(const vector<string> &args, stringstream& ss)
{
    return_value = 0;

    if (args.size()) {
        ShDriver shd(*this);
        string expression;
        int ret;

        merge_string_vec(args, expression, " ");

        ret = shd.parse(expression);
        if (ret) {
            ss << "    invalid expression\n";

            return -1;
        }

        return_value = shd.result;
    }

    return 0;
}

int Shell::help(const vector<string> &args, stringstream& ss)
{
    map<string, HelpEntry>::iterator it;

    if (args.size()) {
        it = help_map.find(args[0]);
        if (it == help_map.end()) {
            ss << " No command named like that\n";
            return -1;
        }
        ss << "   " << it->second.synopsis << ": " << it->second.desc
                << "\n";
    } else {
        ss << "Available commands: (type 'help CMD' to get more info)\n";
        /* Show the synopsis for every command. */
        for (it = help_map.begin(); it != help_map.end(); it++) {
            ss << "       " << it->second.synopsis << "\n";
        }
    }

    return 0;
}

void Shell::history_enable(bool enable)
{
    history_enabled = enable;
}

int Shell::run()
{
    for (;;) {
        string line;
        string token;
        string var;
        vector<string> tokens;
        map<string, ShellCmdFunc>::iterator it;
        stringstream ss;

        if (interactive)
            getline_ncurses(line, "fspcc >> ");
        else
            getline(in, line);

        if (in.eof()) {
            return 0;
        }
        if (in.fail()) {
            cerr << "Shell input error\n";
            return -1;
        }

        istringstream iss(line);

        /* Split the input line into space separated tokens. */
        while (iss >> token) {
            tokens.push_back(token);
        }

        if (!tokens.size()) {
            continue;
        }

        /* Recognize variable assignments (e.g. 'VARNAME = CMD ... ')
           and extract the variable's name. */
        if (tokens.size() >= 3 && tokens[1] == "=") {
            var = tokens[0];
            tokens.erase(tokens.begin());
            tokens.erase(tokens.begin());
        }

        /* Extract the command name. */
        token = tokens[0];
        tokens.erase(tokens.begin());

        /* The "quit" command is never filtered by the if/elif/else/if
           commands. It is therefore useful with the interactive shell
           to quit in whatever moment. */
        if (token == "quit") {
            return 0;
        }

        /* Check if we are accepting commands, looking at the current
           IfFrame instance. The if/elif/else/fi commands are never
           filtered here, because we have nonethless track the evolution
           (ramification) of the branches. */
        if (ifframes.top().accepting || token == "if" || token == "fi" ||
                token  == "elif" || token == "else") {

            /* Command lookup and execution. */
            it = cmd_map.find(token);
            if (it == cmd_map.end()) {
                ss << "	Unrecognized command\n";
            } else {
                ShellCmdFunc fp = it->second;
                int ret;

                ret = (this->*fp)(tokens, ss);

                if (var.size()) {
                    variables[var] = ret;
                }
            }
            /* Flush command output. */
            putsstream(ss, true);

            /* We are at the end of an iteration. If we detect that
               'processes.size()' is changed w.r.t. the last iteration
               (because of the command executed during this iteration),
               we call 'fill_completion()', in order to update the
               AutoCompletion object to the new 'processes' table. */
            if (c.processes.size() != trace_processes_size) {
                trace_processes_size = c.processes.size();
                fill_completion();
            }

            if (token == "exit") {
                return return_value;
            }
        }
    }
}

bool Shell::lookup_variable(const string& name, int& val) const
{
    map<string, int>::const_iterator mit;

    mit = variables.find(name);

    if (mit == variables.end()) {
        return false;
    }
    val = mit->second;

    return true;
}

/* ====================== CommandHistory implementation ================= */
CommandHistory::CommandHistory() {
    /* The history is initialized as containing an empty command string. */
    commands.push_back(string());
    cur = 0;
}

/* Navigate the history backward. */
void CommandHistory::up()
{
    if (cur) {
        cur--;
    }
}

/* Navigate the history forward. */
void CommandHistory::down()
{
    if (cur < commands.size()) {
        cur++;
    }
}

/* Return the current command in the history. */
void CommandHistory::get_current(string& s)
{
    if (cur < commands.size()) {
        s = commands[cur];
    } else /* if (cur == commands.size()) */ {
        s = string();
    }
}

/* Insert a new command into the history. This has always the effect
   of resetting the "current" to the most recent command. The new command
   is actually added to the story only if non-empty and if not equal to
   the most recent command. */
void CommandHistory::add_command(const string& s)
{
    if (s.size() && s != commands.back()) {
        commands.push_back(s);
        if (commands.size() > HISTORY_MAX_COMMANDS) {
            commands.erase(commands.begin());
        }
    } else {
        /* Merge to last entry, if matches. */
    }
    /* Reset the current. */
    cur = commands.size();
}


/* =================== AutoCompletion implementation ===================== */
AutoCompletion::AutoCompletion()
{
    /* The trie data structure initially contains only one node with a NULL
       character. This represents the empty string. */
    trie.push_back(TrieElem('\0', false));
}

/* Insert a string into the trie. */
void AutoCompletion::insert(const string& s)
{
    unsigned int ti = 0;

    /* Scan the input string and the trie in parallel. New nodes are
       inserted into the trie when necessary. */
    for (unsigned int si = 0; si < s.size(); si++) {
        const TrieElem& elem = trie[ti];
        unsigned int j = 0;
        unsigned int n = elem.next.size();

        /* Match the current input character 's[si]' with a trie node
           in the 'next' of the current trie node. */
        for (; j < n; j++) {
            if (trie[ elem.next[j] ].ch == s[si]) {
                /* A match is found: Keep navigating the trie. */
                ti = elem.next[j];
                break;
            }
        }
        if (j == n) {
            /* No match has been found. Create a new trie node containing
               's[si]' and add it to the next of the current trie node. */
            trie.push_back(TrieElem(s[si], si == s.size() - 1));
            trie[ti].next.push_back(trie.size() - 1);
            ti = trie.size() - 1;
        }
    }
}

/* Find the subset of the strings in the trie that 's' is prefix of. If
   the subset is not empty, modify 's' so that is equal to the longest
   common prefix of all the strings in the subset.

   Returns true if 's' has been modified.
*/
bool AutoCompletion::lookup(string& s) const
{
    unsigned int ti = 0;
    bool modified = false;

    /* Scanning similar to the insert() method. */
    for (unsigned int si = 0; si < s.size(); si++) {
        const TrieElem& elem = trie[ti];
        unsigned int j = 0;
        unsigned int n = elem.next.size();

        for (; j < n; j++) {
            if (trie[ elem.next[j] ].ch == s[si]) {
                ti = elem.next[j];
                break;
            }
        }
        if (j == n) {
            /* The input string is not a prefix of any of the strings
               contained into the trie. */
            return modified;
        }
    }

    /* Keep scanning (and updating the input string) the trie as long as
       the sub-trie does not have branches and we don't run into an EOW. */
    for (;;) {
        const TrieElem& elem = trie[ti];

        if (elem.next.size() != 1 || elem.eow) {
            /* The trie is finished or there is a branch: Stop here. */
            break;
        }
        ti = elem.next[0];
        s.push_back(trie[ti].ch);
        modified = true;
    }

    return modified;
}

/* Depth first search visit. Since the trie is a tree structure, it's
   not necessary to keep track of visited nodes or worry about cycles. */
void AutoCompletion::print(unsigned int idx, unsigned int level)
{
    for (unsigned int j = 0; j < level; j++) {
        cout << "  ";
    }
    cout << trie[idx].ch << "\n";

    for (unsigned int j = 0; j < trie[idx].next.size(); j++) {
        this->print(trie[idx].next[j], level + 1);
    }
}

TrieElem::TrieElem(char c, bool end_of_word)
{
    ch = c;
    eow = end_of_word;
}

ShellOption::ShellOption(const string& n, const string& defaul,
                         unsigned int t) : name(n), type(t)
{
    set(defaul);
}

string ShellOption::get() const
{
    return value;
}

int ShellOption::set(const string& val)
{
    switch (type) {
        case Boolean:
            if (val != "y" && val != "n") {
                return -1;
            }
            break;

        case String:
            break;

        default:
            return -1;
    }

    value = val;

    return 0;
}
