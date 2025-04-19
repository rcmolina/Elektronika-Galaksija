rem appmake +gal -b %1.gtp --audio --dumb
gtp2wav -r 4410 -o %1_4410Hz.wav %1.gtp
gtp2wav -r 1470 -o %1_1470Hz.wav %1.gtp