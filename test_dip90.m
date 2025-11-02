% Test dip=90 case
dip = 90;
strike = 90;
msp = [0, 0, -153292.796];

% MATLAB formula from H_total_step_1storder.m line 21
norm = [cosd(90-dip)*cosd(strike-90), sind(strike-90)*cosd(90-dip), sind(90-dip)];
fprintf('dip=90, strike=90:\n');
fprintf('  norm (before normalization) = [%.6f, %.6f, %.6f]\n', norm(1), norm(2), norm(3));

normt = sqrt(dot(norm,norm));
norm = norm/normt;
fprintf('  norm (after normalization) = [%.6f, %.6f, %.6f]\n', norm(1), norm(2), norm(3));

mspdotnorm = dot(msp,norm);
fprintf('  msp.dot(norm) = %.6f\n', mspdotnorm);

msp_new = mspdotnorm * norm;
fprintf('  msp after projection = [%.6f, %.6f, %.6f]\n', msp_new(1), msp_new(2), msp_new(3));
