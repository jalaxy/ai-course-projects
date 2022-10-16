clear;
fig = figure;
for i = 1 : 40
    if i > 28
        imshow([num2str(28), '.jpg']);
    else
        imshow([num2str(i), '.jpg']);
    end
    set(gca, 'position', [0 0 1 1]);
    [A, map] = rgb2ind(frame2im(getframe(fig)), 256);
    if i == 1
        imwrite(A, map, '0.gif', 'gif', 'LoopCount', Inf, 'DelayTime', 0.1);
    else
        imwrite(A, map, '0.gif', 'gif', 'WriteMode', 'append', 'DelayTime', 0.1);
    end
end