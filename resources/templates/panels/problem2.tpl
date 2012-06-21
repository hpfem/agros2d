<html>
  <head>
    <title>FreeCAD - Start page</title>

    <script language="javascript">
    </script>

    <style type="text/css">

        body {
            background: #171A2B url(Background.jpg);
            color: white;
            font-family: Arial, Helvetica, Sans;
            font-size: 11px;
        }

        a {
            color: #0092E8;
            font-weight: bold;
            text-decoration: none;
            padding: 2px;
        }

        a:hover {
            color: white;
            background: #0092E8;
            border-radius: 5px;
        }

        p {
            text-align: justify;
        }

        .left {
            text-align: left;
        }

        h1 {
            font-size: 3em;
            letter-spacing: 2px;
            padding: 20px 0 0 80px;
            align: bottom;
        }

        h2 {
            font-size: 1.2em;
        }

        ul {
            list-style-type: none;
            padding: 0;
        }

        .column {
            width: 300px;
            float: left;
            margin-left: 10px;
        }

        .block {
            background: rgba(30,31,33,0.6);;
            border-radius: 5px;
            padding: 8px;
            margin-bottom: 10px;
        }

        .options {
            clear: both;
        }

        .from {
            font-size: 0.7em;
            font-weight: normal;
        }

    </style>

  </head>

  <body>

    <h1><img src="FreeCAD.png">&nbsp;""" + text01 + """</h1>

    <div class="column">

      <div class="block">
        <h2>""" + text02 + """</h2>
          defaultworkbenches
      </div>

      <div class="block">
        <h2>""" + text03 + """</h2>
          recentfiles
      </div>

      <div class="block">
        <h2>""" + text04 + """ <span class="from">""" + text44 + """</span></h2>
        <div id="youtube">youtube videos</div>
      </div>

      <div class="block">
        <h2>""" + text05 + """ <span class="from">""" + text42 + """</span></h2>
        <div id="news">news feed</div>
      </div>

    </div>

    <div class="column">

      <div class="block">
        <h2>""" + text06 + """</h2>
            defaultlinks
      </div>

      <div class="block">
        <h2>""" + text09 + """</h2>
            defaultexamples
      </div>


      customblocks

    </div>

    <div class="column" id="description">
      &nbsp;
    </div>

    <!--
    <form class="options">
      <input type="checkbox" name="closeThisDialog">
      """ + text17 + """<br/>
      <input type="checkbox" name="dontShowAgain">
      """ + text18 + """
    </form>
    -->

  </body>
</html>
