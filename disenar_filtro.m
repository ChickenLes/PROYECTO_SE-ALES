function [b, a] = disenar_filtro(tipo, clase, Fs, orden, fc1, fc2, metodo)
    if nargin < 7
        metodo = 'butterworth';
    end

    Wn = fc1/(Fs/2);

    if strcmp(clase,'bandpass') || strcmp(clase,'bandstop')
        Wn = [fc1/(Fs/2) fc2/(Fs/2)];
    end

    switch upper(tipo)
        case 'IIR'
            switch lower(metodo)
                case 'butterworth'
                    switch clase
                        case 'lowpass'
                            [b,a] = butter(orden, Wn, 'low');
                        case 'highpass'
                            [b,a] = butter(orden, Wn, 'high');
                        case 'bandpass'
                            [b,a] = butter(orden, Wn, 'bandpass');
                        case 'bandstop'
                            [b,a] = butter(orden, Wn, 'stop');
                    end
                case 'chebyshev'
                    Rp = 1;
                    switch clase
                        case 'lowpass'
                            [b,a] = cheby1(orden, Rp, Wn, 'low');
                        case 'highpass'
                            [b,a] = cheby1(orden, Rp, Wn, 'high');
                        case 'bandpass'
                            [b,a] = cheby1(orden, Rp, Wn, 'bandpass');
                        case 'bandstop'
                            [b,a] = cheby1(orden, Rp, Wn, 'stop');
                    end
            end

        case 'FIR'
            switch lower(metodo)
                case 'damped'
                    switch clase
                        case 'lowpass'
                            b = fir1(orden, Wn, 'low');
                        case 'highpass'
                            b = fir1(orden, Wn, 'high');
                        case 'bandpass'
                            b = fir1(orden, Wn, 'bandpass');
                        case 'bandstop'
                            b = fir1(orden, Wn, 'stop');
                    end
                case 'ls'
                    switch clase
                        case 'lowpass'
                            f_bands = [0 Wn Wn 1];
                            a_bands = [1 1 0 0];
                        case 'highpass'
                            f_bands = [0 Wn Wn 1];
                            a_bands = [0 0 1 1];
                        case 'bandpass'
                            f_bands = [0 Wn(1) Wn(1) Wn(2) Wn(2) 1];
                            a_bands = [0 0 1 1 0 0];
                        case 'bandstop'
                            f_bands = [0 Wn(1) Wn(1) Wn(2) Wn(2) 1];
                            a_bands = [1 1 0 0 1 1];
                    end
                    b = firls(orden, f_bands, a_bands);
            end
            a = 1;
    end
end