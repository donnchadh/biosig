:Begin:
:Function:      sload
:Pattern:       sload[fn_String, i_List]
:Arguments:     {fn, i}
:ArgumentTypes: {String, IntegerList}
:ReturnType:    Manual
:End:


:Evaluate: sload::usage = "sload[filename, {ne,ng,ns}] load data sweeps into mathematica workspace.
 ne, ng, and ns are the number of the experiment, the number of the series from this experiment and
 the number of the sweep from this series sweep, respectivly. 0 can be used as wildcard to select all
 sweeps.\nExamples: sload(\"abc.dat\",{1,5,0}) selects all sweeps from 5th series of first experiment; {0,0,0} selects
 all sweeps from file \"abc.dat\".\nNOTE: If sweeps were sampled with different sampling rates, all data is converted to the
 least common multiple of the various sampling rates. (e.g. loading a 20kHz and a 25kHz sweep simultaneously, both sweeps are converted to 100kHz)." 
 


