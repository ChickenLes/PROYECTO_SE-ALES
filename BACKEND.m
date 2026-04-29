%============================================================================
%Nombre: Anthony Boteo
%Número de carnet: 21061
%Procesamiento de señales
%Sección: 20 
%Proyecto Final - Funciones de backend
%Descripción: funciones para espectro, filtros, resp en frecuencia y efectos
%============================================================================
clc;
clear; 
close all;

%% ==================== PRUEBAS ====================
% Descomentar para ver si funcionan


% Fs = 8000;
% t = (0:(600-1))/Fs;
% x = 5*sin(2*pi*50*t) + 2*sin(2*pi*120*t) + 0.3*sin(2*pi*300*t);
% x = x + 0.2*randn(size(t));  %Ruido
% 
% % --- espectro ---
% [f, P1] = calcular_espectro(x, Fs);
% figure(1);
% plot(f, P1);
% title("Espectro unilateral");
% xlabel("Frecuencia (Hz)");
% ylabel("Amplitud");
% xlim([0 500]);
% grid on;
% 
% % --- filtro IIR lowpass ---
% [b, a] = disenar_filtro("IIR", "lowpass", Fs, 4, 100);
% 
% % --- respuesta en freq ---
% [w_rad, f_hz, H_mag, H_dB, H_fase] = resp_frecuencia(b, a, Fs);
% figure(2);
% subplot(2,1,1);
% plot(f_hz, H_dB);
% title("Magnitud (dB)");
% xlabel("Hz");
% ylabel("dB");
% grid on;
% subplot(2,1,2);
% plot(f_hz, H_fase);
% title("Fase");
% xlabel("Hz");
% ylabel("grados");
% grid on;
% 
% % --- aplicar filtro y comparar ---
% params.b = b;
% params.a = a;
% y_filt = aplicar_efecto(x, Fs, "filtro", params);
% [f2, P2] = calcular_espectro(y_filt, Fs);
% figure(3);
% plot(f, P1, "b");
% hold on;
% plot(f2, P2, "r");
% legend("Original", "Filtrada");
% title("Espectros");
% xlabel("Hz");
% xlim([0 500]);
% grid on;
% 
% % --- eco ---
% p_eco.delay = 0.1;
% p_eco.ganancia = 0.5;
% p_eco.repeticiones = 3;
% y_eco = aplicar_efecto(x, Fs, "eco", p_eco);
% figure(4);
% plot(y_eco);
% title("Con eco");
% grid on;
% 
% % --- distorsion ---
% x_sin = sin(2*pi*50*t);
% p_dist.ganancia = 10;
% p_dist.umbral = 0.3;
% y_dist = aplicar_efecto(x_sin, Fs, "distorsion", p_dist);
% figure(5);
% subplot(2,1,1);
% plot(t, x_sin);
% title("Original");
% grid on;
% subplot(2,1,2);
% plot(t, y_dist, "r");
% title("Distorsion");
% grid on;


%% ===================== FUNCIONES =====================

function [f, P1] = calcular_espectro(senal, Fs)
% Espectro de amplitud unilateral  
% Igual que en el lab 5 y 11

    senal = senal(:);
    N = length(senal);
    
    X = fft(senal);
    X_amp = abs(X)/N;
    
    % Espectro unilateral
    P1 = X_amp(1:floor(N/2)+1);
    P1(2:end-1) = 2*P1(2:end-1);
    
    f = (0:floor(N/2))' * (Fs/N);  %freq en Hz
end


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


function [w_rad, f_hz, H_mag, H_dB, H_fase] = resp_frecuencia(b, a, Fs)
% respuesta en frecuencia, como en lab 11 parte 4

    [H, w] = freqz(b, a, 1024);
    
    w_rad = w;
    f_hz = (w/pi) * (Fs/2);       %convertir a Hz 
    H_mag = abs(H);
    H_dB = 20*log10(abs(H)+eps);
    H_fase = angle(H) * (180/pi);  %grados
end


function y = aplicar_efecto(senal, Fs, tipo_efecto, parametros)
% para "filtro": parametros.b y parametros.a
% para "eco": parametros.delay, parametros.ganancia, parametros.repeticiones
% para "distorsion": parametros.ganancia, parametros.umbral

    senal = senal(:);
    
    switch tipo_efecto
        case "filtro"
            y = filter(parametros.b, parametros.a, senal);
            
        case "eco"
            % y[n] = x[n] + g*x[n-D] + g^2*x[n-2D]...
            D = round(parametros.delay * Fs);
            g = parametros.ganancia;
            reps = parametros.repeticiones;
            
            b_eco = zeros(1, reps*D+1);
            b_eco(1) = 1;  %senal original
            for k = 1:reps
                b_eco(k*D+1) = g^k;
            end
            y = filter(b_eco, 1, senal);
            
        case "distorsion"
            % Hard clipping 
            gain = parametros.ganancia;
            umb = parametros.umbral;
            
            mx = max(abs(senal));
            if mx > 0
                s = senal/mx;
            else
                s = senal;
            end
            s = gain*s;
            
            %recortar
            for i = 1:length(s)
                if s(i) > umb
                    s(i) = umb;
                elseif s(i) < -umb
                    s(i) = -umb;
                end
            end
            y = (s/umb) * mx;
    end
end