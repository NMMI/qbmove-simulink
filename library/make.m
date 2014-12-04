
if(isempty(dir('bin')))
    mkdir(pwd, 'bin');
end

mex -outdir bin qbmoveInit.cpp ../../qbAPI/src/qbmove_communications.cpp
mex -outdir bin qbmove.cpp ../../qbAPI/src/qbmove_communications.cpp
mex -outdir bin qbcurrent.cpp ../../qbAPI/src/qbmove_communications.cpp
mex -outdir bin c_gui_utils.cpp ../../qbAPI/src/qbmove_communications.cpp