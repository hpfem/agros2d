clc;

MR=[0,  0.1255; 
    0.5, 0.9765;
    1,  0.96];
MG=[0, 0.1622; 
    0.5, 0.8745;
    1, 0.1804];
MB=[0,  0.96; 
    0.5, 0.3020;
    1, 0.1490];
                  
cmap = agros_colormap(256, MR, MG, MB);

% cmap = gray(11);
for i = 1:length(cmap)
    fprintf('{ %f, %f, %f }, \n', cmap(i,1), cmap(i,2), cmap(i,3));
end

