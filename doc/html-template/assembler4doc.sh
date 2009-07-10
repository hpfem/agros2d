#/bin/sh

TemplateDirectory="./template/"
#OutputDirectory="./`date +%d.%m.%y`/"
OutputDirectory="../html/"

HeaderFile="./header.src"
FooterFile="./footer.src"

function Assembly()
{
  if find $OutputDirectory"/"$ActivFile".html"  2> /dev/null > /dev/null
  then
    if !(rm $OutputDirectory"/"$ActivFile".html")
    then
      exit 1
    fi
  fi

  if cat $HeaderFile >> $OutputDirectory$ActivFile".html" && cat $TemplateDirectory$ActivFile".src" >> $OutputDirectory$ActivFile".html" && cat $FooterFile >> $OutputDirectory$ActivFile".html"
  then
    return 0
  else
    return 1
  fi
}

rm -r $OutputDirectory
if !(find $OutputDirectory  2> /dev/null > /dev/null)
then
  if !(mkdir $OutputDirectory)
  then
    exit 1
  fi
fi

if (cp -r $TemplateDirectory/* $OutputDirectory)
then
  if !(rm $OutputDirectory"/"*.src)
  then
    exit 1
  fi
else
  exit 1
fi

for ActivFile in $TemplateDirectory*".src"
do
  ActivFile=`basename $ActivFile ".src"`
  Assembly

echo $OutputDirectory$ActivFile".html"
done
