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