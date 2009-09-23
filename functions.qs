// geometry functions
function addCircle(x0, y0, radius, marker)
{
  if (marker == null)
  {
    marker = 'none';
  }

  addEdge(x0-radius, y0, x0+radius, y0, 180, marker);
  addEdge(x0+radius, y0, x0-radius, y0, 180, marker);

  return;
}

function addRect(x0, y0, width, height, marker)
{
  if (marker == null)
  {
    marker = 'none';
  }

  addEdge(x0, y0, x0+width, y0, 0, marker);
  addEdge(x0+width, y0, x0+width, y0+height, 0, marker);
  addEdge(x0+width, y0+height, x0, y0+height, 0, marker);
  addEdge(x0, y0+height, x0, y0, 0, marker);

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
