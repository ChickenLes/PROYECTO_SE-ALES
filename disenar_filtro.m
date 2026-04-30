function [b, a] = disenar_filtro(tipo, clase, Fs, orden, fc1, fc2)
% Tipo: "IIR" o "FIR"
% Clase: "lowpass" "highpass" "bandpass" "bandstop"
% fc2 solo se usa para bandpass y bandstop

    Wn = fc1/(Fs/2);  %normalizar
    
    %bandpass y bandstop necesitan 2 frecuencias
    if strcmp(clase,"bandpass") || strcmp(clase,"bandstop")
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
                case "bandpass"
                    [b,a] = butter(orden, Wn, "bandpass");
                case "bandstop"
                    [b,a] = butter(orden, Wn, "stop");
            end
        case "FIR"
            % fir1
            switch clase
                case "lowpass"
                    b = fir1(orden, Wn, "low");
                case "highpass"
                    b = fir1(orden, Wn, "high");
                case "bandpass"
                    b = fir1(orden, Wn, "bandpass");
                case "bandstop"
                    b = fir1(orden, Wn, "stop");
            end
            a = 1;
    end
end