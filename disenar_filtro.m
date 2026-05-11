function [b, a] = disenar_filtro(tipo, clase, Fs, orden, fc1, fc2)
% Tipo: "IIR" o "FIR"
% Clase: "lowpass" "highpass" "BANDPASS" "BANDSTOP"
% fc2 solo se usa para BANDPASS y BANDSTOP

    Wn = fc1/(Fs/2);  %normalizar
    
    %BANDPASS y BANDSTOP necesitan 2 frecuencias
    if strcmp(clase,"BANDPASS") || strcmp(clase,"BANDSTOP")
        Wn = [fc1/(Fs/2) fc2/(Fs/2)];
    end
    
    switch upper(tipo)
        case "IIR"
            % butterworth
            switch clase
                case "lowpass"
                    [b,a] = butter(orden, Wn, "low");
                case "highpass"
                    [b,a] = butter(orden, Wn, "high");
                case "BANDPASS"
                    [b,a] = butter(orden, Wn, "BANDPASS");
                case "BANDSTOP"
                    [b,a] = butter(orden, Wn, "stop");
            end
        case "FIR"
            % fir1
            switch clase
                case "lowpass"
                    b = fir1(orden, Wn, "low");
                case "highpass"
                    b = fir1(orden, Wn, "high");
                case "BANDPASS"
                    b = fir1(orden, Wn, "BANDPASS");
                case "BANDSTOP"
                    b = fir1(orden, Wn, "stop");
            end
            a = 1;
    end
end