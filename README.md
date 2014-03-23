fspc
====

An FSP (Finite State Process) compiler + an LTS (Labeled Transition System) analysis tool.



FSPC(1)                            fspc manual                           FSPC(1)


NAME
       fspc - An FSP compiler and LTS analysis tool


SYNOPSIS
       fspcc [-dpgasvh] [-S FILE] [-D NUM] -i FILE [-o FILE]
       fspcc [-dpgasvh] [-S FILE] [-D NUM] -l FILE



DESCRIPTION
       This  command  invokes  an  Finite  State  Process  (FSP)  compiler and a
       Labelled Transition System (LTS) analysis tool.

       The FSP language can be used to model concurrent  systems,  e.g.   multi-
       threaded/multi-process  applications. An FSP model can be compiled into a
       graph-based representation using an FSP compiler:  each  FSP  process  is
       transformed into an equivalent LTS.

       An LTS analysis tool is then able to:

             -   detect  concurrency  problems  in  the  concurrent system (e.g.
                 deadlock, starvation)

             -   show that certain properties (or assertions) hold on  the  con‐
                 current system

             -   run a simulation (animation) of the concurrent system, in order
                 to gain confidence on the model

       When invoked in the first form, you use the -i option to specify an input
       file containing FSP definitions. The tool will compile all the FSP models
       and produce an output file containing the corresponding compiled LTSs.

       When invoked in the second form, you use the -l option to specify a  file
       containing  a set of compiled LTSs, in order to use the LTS analysis tool
       without compiling again.



OPTIONS
       -i PATHNAME
          Specifies an input file pathname containing FSP  definitions  to  com‐
          pile.


       -o PATHNAME
          Specifies  the output file pathname where compiled LTSs are stored. If
          this option is not specified, the default output file  name  is  'out‐
          put.lts'.


       -l PATHNAME
          Specifies the input file pathname containing compiled LTSs.


       -d
          Runs  deadlock/error  analysis  on every compiled LTS. A list of dead‐
          locks/errors will be printed on the standard output. If there  are  no
          such problems, no output is produced.


       -p
          Runs  all  the specified progress checks on every compiled LTS. A list
          of progress violations will be printed  on  the  standard  output.  If
          there are no such problems, no output is produced.


       -g
          Outputs  a  GraphViz  representation  file corrisponding to every LTS.
          For each process named 'P', a file called 'P.gv' is created.


       -a
          The same as -dpg.


       -s
          Runs an LTS analysis interactive shell. The shell is run after  compi‐
          lation (if any).


       -S PATHNAME
          Runs  the LTS analysis script specified by the pathname. The script is
          run after the compilation (if any) but before invoking the interactive
          shell (when -s is specified).


       -D NUMBER
          Specifies  the  maximum accepted depth for process references. This is
          mainly useful to avoid problems with  poorly  written  recursive  pro‐
          cesses.   Example  of a problematic process, which causes unterminated
          recursion:

              P(K=3) = if (K == 0) then END else P(K-1);P(K-2);END.


       -v
          Show versioning information.


       -h
          Show the help.



EXAMPLES
       Here is a simple example of FSP input file (say in.fsp):

           P = (a->b->P|c->d->END).
           Q = (t[i:1..2]-> (when (i>1) u->Q | v->Q)).
           ||C = (P || Q).

       Compile these FSP models and store the result into 'out.lts'

           fspcc -i in.fsp -o out.lts

       Now run all the analysis and produce GraphViz representations

           fspcc -a -l out.lts

       Alternatively, you can do the same thing with just one command

           fspcc -a -i in.fsp -o out.lts

       To launch an interactive shell

           fspcc -s -i in.fsp

       or, from a compiled LTS file

           fspcc -s -l out.lts



AUTHOR
       Written by Vincenzo Maffione.


REPORTING BUGS
       Please report fspcc bugs to v.maffione@gmail.com.
       See fspcc project homepage: <http://www.bitbucket.org/vmaffione/fspc>


COPYRIGHT
       Copyright © 2013 Vincenzo Maffione.  License GPLv3+: GNU GPL version 3 or
       later <http://gnu.org/licenses/gpl.html>.
       This is free software: you are free to change and redistribute it.  There
       is NO WARRANTY, to the extent permitted by law.

