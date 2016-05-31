if exist(strcat(pwd, '/bin'), 'file')
    rmdir('bin', 's');
else
	mkdir(pwd, 'bin');
end

mex -outdir bin src/qbmoveInit.cpp ../../qbAPI/src/qbmove_communications.cpp -Iinclude/
mex -outdir bin src/qbmove.cpp ../../qbAPI/src/qbmove_communications.cpp -Iinclude/
mex -outdir bin src/qbcurrent.cpp ../../qbAPI/src/qbmove_communications.cpp -Iinclude/
mex -outdir bin src/qbemg.cpp ../../qbAPI/src/qbmove_communications.cpp -Iinclude/
mex -outdir bin src/qbmeascurr.cpp ../../qbAPI/src/qbmove_communications.cpp -Iinclude/