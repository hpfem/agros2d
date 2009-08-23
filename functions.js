function sqr(x) 
{ 
    return x*x; 
}

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
