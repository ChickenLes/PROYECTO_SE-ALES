function [w_rad, f_hz, H_mag, H_dB, H_fase] = resp_frecuencia(b, a, Fs)
% respuesta en frecuencia, como en lab 11 parte 4

    [H, w] = freqz(b, a, 1024);
    
    w_rad = w;
    f_hz = (w/pi) * (Fs/2);       %convertir a Hz (igual que lab 11)
    H_mag = abs(H);
    H_dB = 20*log10(abs(H)+eps);
    H_fase = angle(H) * (180/pi);  %grados
end