rem sox %1.wav %1.voc
rem direct /t 79 %1.voc %1_id15.tzx
rem tzx2wav -f 1470 %1_id15.tzx
sox %1.wav %1.voc
direct /t 790 %1.voc %1_id15.tzx
tzx2wav -f 4405 %1_id15.tzx
direct /t 2370 %1.voc %1_1470Hz_id15.tzx
tzx2wav -f 1477 %1_1470Hz_id15.tzx
