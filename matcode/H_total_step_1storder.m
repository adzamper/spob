function [ H_tot_x,H_tot_y,H_tot_z,H_x,H_y,H_z,convo_x,convo_z ] = H_total_step_1storder( mtx,dipoleM,rtx,rtxrx,rsp,t,mu,sigma_ob,thick_ob,sigma_sp,a,applydip,dip,strike,T )
%this function calculates the sphere-overburden response in second order

% DEBUG: Print for dip=45 case
if (applydip==1 && abs(dip-45.0)<0.1)
    fprintf('\n=== MATLAB DEBUG: dip=%g case ===\n', dip);
    fprintf('INPUT PARAMETERS:\n');
    fprintf('  rtx = [%g, %g, %g]\n', rtx(1), rtx(2), rtx(3));
    fprintf('  rtxrx = [%g, %g, %g]\n', rtxrx(1), rtxrx(2), rtxrx(3));
    fprintf('  rsp = [%g, %g, %g]\n', rsp(1), rsp(2), rsp(3));
    fprintf('  dip = %g, strike = %g\n', dip, strike);
    fprintf('  a = %g, sigma_sp = %g, sigma_ob = %g, thick_ob = %g\n', a, sigma_sp, sigma_ob, thick_ob);
    fprintf('  t = %.6e, T = %g\n', t, T);
end

temp=2*pi*a^3*dH_tot_x_step( mtx,dipoleM,[0,0,rtx(3)],[-rtx(1),-rtx(2),rsp(3)],mu,sigma_ob,thick_ob,t,0,sigma_sp,a,T );
%store x component of induced moment
convo_x=temp;
convo_y=0;
H_tot_y=0;
H_y=0;
%This loop convolves with the tx waveform for z component
temp=2*pi*a^3*dH_tot_z_step( mtx,dipoleM,[0,0,rtx(3)],[-rtx(1),-rtx(2),rsp(3)],mu,sigma_ob,thick_ob,t,0,sigma_sp,a,T );
%store z component of induced moment
convo_z=temp;

%store sphere moment
msp=[convo_x,convo_y,convo_z];

% DEBUG: Print induced moment
if (applydip==1 && abs(dip-45.0)<0.1)
    fprintf('INDUCED MOMENT (before dip):\n');
    fprintf('  convo_x = %.10e\n', convo_x);
    fprintf('  convo_z = %.10e\n', convo_z);
    fprintf('  msp = [%.10e, %.10e, %.10e]\n', msp(1), msp(2), msp(3));
end

%dipping sphere model if applydip=1
if (applydip==1)
  norm=[cosd(90.-dip)*cosd(strike-90.),sind(strike-90.)*cosd(90.-dip),sind(90.-dip)];
% make the dip normal vector a unit vector
  normt=sqrt(dot (norm,norm));
  norm=norm/normt;

  % DEBUG: Print normal vector calculation
  if (abs(dip-45.0)<0.1)
      fprintf('NORMAL VECTOR CALCULATION:\n');
      fprintf('  90-dip = %g degrees\n', 90-dip);
      fprintf('  strike-90 = %g degrees\n', strike-90);
      fprintf('  norm (before normalize) = [%.10e, %.10e, %.10e]\n', ...
              cosd(90.-dip)*cosd(strike-90.), sind(strike-90.)*cosd(90.-dip), sind(90.-dip));
      fprintf('  normt = %.10e\n', normt);
      fprintf('  norm (after normalize) = [%.10e, %.10e, %.10e]\n', norm(1), norm(2), norm(3));
  end
end
if (applydip==1)
  mspdotnorm=dot(msp,norm);
% now scale the normal to have this strength and redirect the sphere
% moment to be in the dip direction
  msp_before_proj = msp;
  msp=mspdotnorm*norm;

  % DEBUG: Print projection
  if (abs(dip-45.0)<0.1)
      fprintf('MOMENT PROJECTION:\n');
      fprintf('  msp (before) = [%.10e, %.10e, %.10e]\n', msp_before_proj(1), msp_before_proj(2), msp_before_proj(3));
      fprintf('  mspdotnorm = %.10e\n', mspdotnorm);
      fprintf('  msp (after) = [%.10e, %.10e, %.10e]\n', msp(1), msp(2), msp(3));
  end
end

% Compute relative position vector
r_rel = [-rtxrx(1),-rtxrx(2),rtx(3)-rtxrx(3)]-[-rtx(1),-rtx(2),rsp(3)];

% DEBUG: Print position vectors
if (applydip==1 && abs(dip-45.0)<0.1)
    fprintf('POSITION VECTORS:\n');
    fprintf('  r_receiver = [%.10e, %.10e, %.10e]\n', -rtxrx(1), -rtxrx(2), rtx(3)-rtxrx(3));
    fprintf('  r_sphere = [%.10e, %.10e, %.10e]\n', -rtx(1), -rtx(2), rsp(3));
    fprintf('  r_rel = [%.10e, %.10e, %.10e]\n', r_rel(1), r_rel(2), r_rel(3));
end

%calculate field using induced moment
H_sphere = static(msp,r_rel);

% DEBUG: Print static field
if (applydip==1 && abs(dip-45.0)<0.1)
    fprintf('STATIC DIPOLE FIELD:\n');
    fprintf('  H_sphere = [%.10e, %.10e, %.10e]\n', H_sphere(1), H_sphere(2), H_sphere(3));
end

H_tot_x=-dot([1,0,0],H_sphere);
H_tot_z=dot([0,0,1],H_sphere);

% DEBUG: Print sphere contribution
if (applydip==1 && abs(dip-45.0)<0.1)
    fprintf('SPHERE CONTRIBUTION:\n');
    fprintf('  H_tot_x (from sphere) = %.10e\n', H_tot_x);
    fprintf('  H_tot_z (from sphere) = %.10e\n', H_tot_z);
end

%calculate 0th order term (field of overburden alone) and convolve with
%waveform
H_x=H_ob_x( mtx,dipoleM,[0,0,rtx(3)],[-rtxrx(1),-rtxrx(2),rtx(3)-rtxrx(3)],t,mu,sigma_ob,thick_ob );
H_tot_x=H_tot_x+H_x;

H_z=H_ob_z( mtx,dipoleM,[0,0,rtx(3)],[-rtxrx(1),-rtxrx(2),rtx(3)-rtxrx(3)],t,mu,sigma_ob,thick_ob );
H_tot_z=H_tot_z+H_z;

% DEBUG: Print final values
if (applydip==1 && abs(dip-45.0)<0.1)
    fprintf('OVERBURDEN CONTRIBUTION:\n');
    fprintf('  H_x = %.10e\n', H_x);
    fprintf('  H_z = %.10e\n', H_z);
    fprintf('FINAL TOTAL FIELD:\n');
    fprintf('  H_tot_x = %.10e\n', H_tot_x);
    fprintf('  H_tot_z = %.10e\n', H_tot_z);
    fprintf('=== END MATLAB DEBUG ===\n\n');
end