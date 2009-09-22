// geometry functions
function rect(x0, y0, l, h, marker)
{
  addEdge(x0, y0, x0+l, y0, 0, marker);
  addEdge(x0+l, y0, x0+l, y0+h, 0, marker);
  addEdge(x0+l, y0+h, x0, y0+h, 0, marker);
  addEdge(x0, y0+h, x0, y0, 0, marker);
  return;
}

// mathematical functions
function sqr(x)
{
  return x*x;
}

// other functions
function step(x, border, value)
{
  return (x <= border) ? 0.0 : value;
}

function table(x, xval, yval)
{
  for (var i = 0; i<xval.length-1; i++)
  {
    if ((x > xval[i]) && (x <= xval[i+1]))
    {
      return (yval[i]+(yval[i+1]-yval[i])*(x-xval[i])/(xval[i+1]-xval[i]));
    }
  }
}
