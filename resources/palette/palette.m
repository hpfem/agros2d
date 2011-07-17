clc;

cmap = gray(11);

for i = 1:length(cmap)
    fprintf('{ %f, %f, %f }, \n', cmap(i,1), cmap(i,2), cmap(i,3));
end

