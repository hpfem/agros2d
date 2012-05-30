function mp = agros_colormap(N, rm, gm, bm)
  x = linspace(0,1, N);
  rv = interp1( rm(:,1), rm(:,2), x);
  gv = interp1( gm(:,1), gm(:,2), x);
  mv = interp1( bm(:,1), bm(:,2), x);
  
  mp = [ rv', gv', mv'];

  mp(isnan(mp)) = 0;
  mp((mp > 1)) = 1;
  mp((mp < 0)) = 0;
end