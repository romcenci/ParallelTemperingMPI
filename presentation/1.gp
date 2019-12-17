set terminal qt font ",12"
set key font ",12"
set xtics 4,2

# Execution time
set logscale
set key top right
set xlabel "Number of ranks"
set ylabel "Execution time (seconds)"
p [][10:2000] for[i in "100 200 400"] 'Task'.i.'.dat' u 1:3 w lp pt 7 ps 1.1 lw 2 t i.' tasks'

# Speedup
set logscale x
set key top left
set xlabel "Number of ranks"
set ylabel "Speedup"
p [4:][0:270] 'Task100.dat' u 1:(4*273.579/$3) w lp pt 7 ps 1.1 lw 2 t '100 tasks', 'Task200.dat' u 1:(4*542.334/$3) w lp pt 7 ps 1.1 lw 2 t '200 tasks', 'Task400.dat' u 1:(4*1250.254/$3) w lp pt 7 ps 1.1 lw 2 t '400 tasks'

# Parallel efficiencies
set xlabel "Number of ranks"
set ylabel "Parallel efficiency"
set xtics 32
p [][0.7:1.6] 'Task100.dat' u 1:(4*273.579/($1*$3)) w lp pt 7 ps 1.1 lw 2 t '100 tasks', 'Task200.dat' u 1:(4*542.334/($1*$3)) w lp pt 7 ps 1.1 lw 2 t '200 tasks', 'Task400.dat' u 1:(4*1250.254/($1*$3)) w lp pt 7 ps 1.1 lw 2 t '400 tasks'