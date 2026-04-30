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