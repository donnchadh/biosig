



<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<!-- ViewVC :: http://www.viewvc.org/ -->
<head>
<title>SourceForge.net Repository - [octave] Log of /trunk/octave-forge/extra/NaN/inst/train_sc.m</title>
<meta name="generator" content="ViewVC 1.1.6" />
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<meta name="description" content="The world's largest development and download repository of Open Source code and applications" />
<meta name="keywords" content="Open Source, Development, Developers, Projects, Downloads, OSTG, VA Software, SF.net, SourceForge" />
<title>SourceForge.net Repository - octave Index of /</title>
<link rel="shortcut icon" href="http://a.fsdn.com/con/img/favicon.ico" />
<link rel="stylesheet" href="http://a.fsdn.com/con/css/sf.min.css?1256826599" type="text/css" />
<link rel="stylesheet" href="/viewvc-static/styles.css" type="text/css" />
<style type="text/css">
#doc3 { max-width: 1000px; margin: 0 auto; }
#hd .logo {
background: url("http://a.fsdn.com/sf/images/phoneix/sf_phoneix.png") no-repeat;
color: transparent;
display: inline-block;
height: 36px;
}
#hd .search { display: none; }
#breadcrumbs { margin-bottom: 1em; }
#fad83 { float: right; }
#yui-main { min-height: 100px; }
#ft .yui-u { display: inline-block; margin-right: 2em; }
</style>

<!-- BEGIN: AdSolution-Tag 4.2: Global-Code [PLACE IN HTML-HEAD-AREA!] -->
<!-- DoubleClick Random Number -->
<script language="JavaScript" type="text/javascript">
dfp_ord=Math.random()*10000000000000000;
dfp_tile = 1;
</script>
<!-- End DoubleClick Random Number -->
<script type="text/javascript">
var google_page_url = 'http://sourceforge.net/projects/octave/';
var sourceforge_project_name = 'octave';
var sourceforge_project_description = '';
</script>
<!-- END: AdSolution-Tag 4.2: Global-Code -->
<!-- after META tags -->
<script type="text/javascript">
var gaJsHost = (("https:" == document.location.protocol) ? "https://ssl." : "http://www.");
document.write(unescape("%3Cscript src='" + gaJsHost + "google-analytics.com/ga.js' type='text/javascript'%3E%3C/script%3E"));
</script>
</head>
<body class="short-head">
<!--[if IE 7]><div id="ie7only"><![endif]-->
<!--[if IE 6]><div id="ie6only"><![endif]-->
<!--[if IE]><div id="ieonly"><![endif]-->
<div id="doc3">
<div id="hd">
<div class="yui-b">
<div class="yui-gc">
<div class="yui-u first">
<a href="http://sourceforge.net/" class="logo">Find and develop open source software</a>
</div>
<div class="yui-u">
<div class="search">
<form action="http://sourceforge.net/search/" method="get" name="searchform" id="searchform">
<input type="hidden" name="type_of_search" value="soft" />
<input type="text" class="text clear hint" name="words" id="words" value="enter keyword" />
<span class="yui-button yui-push-button default"><span class="first-child"><button type="submit" value="Search">Search</button></span></span>
</form>
</div>
</div>
</div>
</div>
</div>
<div id="bd" class="inner-bd">
<div id="doc4" class="yui-t6">
<div id="breadcrumbs">

<a href="http://sourceforge.net/">SourceForge.net</a>
&gt; <a href="http://sourceforge.net/softwaremap/">Find Software</a>
&gt; <a href="http://sourceforge.net/projects/octave/">octave</a>
&gt; SCM Repositories


<a href="/viewvc/octave/">


&gt; octave


</a>



<a href="/viewvc/octave/trunk/">


&gt; trunk


</a>



<a href="/viewvc/octave/trunk/octave-forge/">


&gt; octave-forge


</a>



<a href="/viewvc/octave/trunk/octave-forge/extra/">


&gt; extra


</a>



<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/">


&gt; NaN


</a>



<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/">


&gt; inst


</a>




&gt; train_sc.m




</div>










<div id="project_nav_container">
<small><div>

<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/"><img src="/viewvc-static/images/back_small.png" width="16" height="16" alt="Parent Directory" /> Parent Directory</a>

| <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?view=log"><img src="/viewvc-static/images/log.png" width="16" height="16" alt="Revision Log" /> Revision Log</a>




</div>
</small>
<h1>SCM Repositories - <a href="http://sourceforge.net/projects/octave">octave</a></h1>
</div>
<hr class="clear">
<div class="yui-b">
<div id="fad83">
<!-- DoubleClick Ad Tag -->
<script type="text/javascript">
//<![CDATA[
document.write('<script src="http://ad.doubleclick.net/adj/ostg.sourceforge/pg_viewvc_p88_shortrec;pg=viewvc;tile='+dfp_tile+';tpc=octave;ord='+dfp_ord+';sz=1x1?" type="text/javascript"><\/script>');
dfp_tile++;
//]]>
</script>
<!-- End DoubleClick Ad Tag -->
</div>
</div>
<div id="yui-main">
<div class="yui-b sfBox">
<table class="auto">



<tr>
<td>Links to HEAD:</td>
<td>

(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8314&amp;view=markup">view</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8314">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8314&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=8314">annotate</a>)
mime-type: text/x-objective-c
</td>
</tr>



<tr>
<td>Sticky Revision:</td>
<td><form method="get" action="/viewvc/octave" style="display: inline">
<div style="display: inline">
<input type="hidden" name="orig_pathtype" value="FILE"/><input type="hidden" name="orig_view" value="log"/><input type="hidden" name="orig_path" value="trunk/octave-forge/extra/NaN/inst/train_sc.m"/><input type="hidden" name="view" value="redirect_pathrev"/>

<input type="text" name="pathrev" value="" size="6"/>

<input type="submit" value="Set" />
</div>
</form>

</td>
</tr>
</table>
 


</div>
</div>
<hr class="clear">
<div>






<div>
<hr />

<a name="rev8314"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=8314"><strong>8314</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8314&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8314">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8314&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=8314">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=8314&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Mon Jun  6 14:20:21 2011 UTC</em>
(73 minutes, 45 seconds ago)
by <em>schloegl</em>









<br />File length: 38367 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=8223&amp;r2=8314">previous 8223</a>







<pre class="vc_log">change default settings for usage of bioinfo-tb based on user feedback</pre>
</div>



<div>
<hr />

<a name="rev8223"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=8223"><strong>8223</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8223&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8223">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8223&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=8223">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=8223&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Wed Apr 20 09:16:06 2011 UTC</em>
(6 weeks, 5 days ago)
by <em>schloegl</em>









<br />File length: 38060 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=8075&amp;r2=8223">previous 8075</a>







<pre class="vc_log">update contact e-mail and www address</pre>
</div>



<div>
<hr />

<a name="rev8075"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=8075"><strong>8075</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8075&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8075">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=8075&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=8075">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=8075&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Thu Jan 27 17:10:36 2011 UTC</em>
(4 months, 1 week ago)
by <em>schloegl</em>









<br />File length: 38055 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=7776&amp;r2=8075">previous 7776</a>







<pre class="vc_log">fix web address</pre>
</div>



<div>
<hr />

<a name="rev7776"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=7776"><strong>7776</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7776&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7776">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7776&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=7776">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=7776&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Mon Sep 27 07:55:11 2010 UTC</em>
(8 months, 1 week ago)
by <em>schloegl</em>









<br />File length: 38053 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=7756&amp;r2=7776">previous 7756</a>







<pre class="vc_log">fix SVM for data with missing values (deletion mode)</pre>
</div>



<div>
<hr />

<a name="rev7756"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=7756"><strong>7756</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7756&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7756">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7756&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=7756">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=7756&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Sun Sep 19 21:33:31 2010 UTC</em>
(8 months, 2 weeks ago)
by <em>schloegl</em>









<br />File length: 38051 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=7518&amp;r2=7756">previous 7518</a>







<pre class="vc_log">remove obsolete test for weight vector in LibLinear</pre>
</div>



<div>
<hr />

<a name="rev7518"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=7518"><strong>7518</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7518&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7518">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7518&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=7518">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=7518&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Thu Aug 12 21:02:22 2010 UTC</em>
(9 months, 3 weeks ago)
by <em>schloegl</em>









<br />File length: 38302 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=7378&amp;r2=7518">previous 7378</a>







<pre class="vc_log">fix for single column classlabel with elements {-1,1}</pre>
</div>



<div>
<hr />

<a name="rev7378"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=7378"><strong>7378</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7378&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7378">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=7378&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=7378">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=7378&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Tue Jun  1 19:00:29 2010 UTC</em>
(12 months ago)
by <em>schloegl</em>









<br />File length: 38290 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6973&amp;r2=7378">previous 6973</a>







<pre class="vc_log">row_column_deletion algorithm to exclude missing values is now more flexible</pre>
</div>



<div>
<hr />

<a name="rev6973"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6973"><strong>6973</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6973&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6973">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6973&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6973">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6973&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Sun Feb 28 20:19:12 2010 UTC</em>
(15 months ago)
by <em>schloegl</em>









<br />File length: 39527 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6803&amp;r2=6973">previous 6803</a>







<pre class="vc_log">update website, svn keywords, prepare for next release</pre>
</div>



<div>
<hr />

<a name="rev6803"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6803"><strong>6803</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6803&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6803">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6803&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6803">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6803&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Wed Jan 27 19:26:32 2010 UTC</em>
(16 months, 1 week ago)
by <em>schloegl</em>









<br />File length: 39413 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6748&amp;r2=6803">previous 6748</a>







<pre class="vc_log">several LDA improvements: ill-defined covariances better supported, a litte speed-up</pre>
</div>



<div>
<hr />

<a name="rev6748"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6748"><strong>6748</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6748&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6748">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6748&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6748">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6748&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Mon Jan 11 20:47:51 2010 UTC</em>
(16 months, 3 weeks ago)
by <em>schloegl</em>









<br />File length: 39461 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6742&amp;r2=6748">previous 6742</a>







<pre class="vc_log">weighted libLinear supported</pre>
</div>



<div>
<hr />

<a name="rev6742"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6742"><strong>6742</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6742&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6742">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6742&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6742">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6742&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Mon Jan 11 10:17:20 2010 UTC</em>
(16 months, 3 weeks ago)
by <em>schloegl</em>









<br />File length: 39419 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6739&amp;r2=6742">previous 6739</a>







<pre class="vc_log">PSVM improved/fixed; compilation of libsvm for matlab improved; silence libsvm </pre>
</div>



<div>
<hr />

<a name="rev6739"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6739"><strong>6739</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6739&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6739">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6739&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6739">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6739&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Sun Jan 10 23:25:59 2010 UTC</em>
(16 months, 3 weeks ago)
by <em>schloegl</em>









<br />File length: 39419 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6734&amp;r2=6739">previous 6734</a>







<pre class="vc_log">add support for non-linear SVM and 1-1 scheme</pre>
</div>



<div>
<hr />

<a name="rev6734"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6734"><strong>6734</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6734&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6734">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6734&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6734">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6734&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Sun Jan 10 21:39:00 2010 UTC</em>
(16 months, 3 weeks ago)
by <em>schloegl</em>









<br />File length: 39415 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6727&amp;r2=6734">previous 6727</a>







<pre class="vc_log">Support Vector Machine (SVM) added </pre>
</div>



<div>
<hr />

<a name="rev6727"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6727"><strong>6727</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6727&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6727">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6727&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6727">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6727&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Sun Jan 10 00:13:09 2010 UTC</em>
(16 months, 3 weeks ago)
by <em>schloegl</em>









<br />File length: 39238 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6722&amp;r2=6727">previous 6722</a>







<pre class="vc_log">{-1,+1} encoding of classlabel supported; test of liblinear and libSVM with Octave3.2</pre>
</div>



<div>
<hr />

<a name="rev6722"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6722"><strong>6722</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6722&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6722">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6722&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6722">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6722&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Fri Jan  8 22:12:45 2010 UTC</em>
(16 months, 3 weeks ago)
by <em>schloegl</em>









<br />File length: 35758 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6720&amp;r2=6722">previous 6720</a>







<pre class="vc_log">more clean-up: replace error(sprintf(.)) with error(.); determining DIM </pre>
</div>



<div>
<hr />

<a name="rev6720"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6720"><strong>6720</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6720&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6720">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6720&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6720">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6720&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Fri Jan  8 12:49:59 2010 UTC</em>
(16 months, 3 weeks ago)
by <em>schloegl</em>









<br />File length: 35991 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6719&amp;r2=6720">previous 6719</a>







<pre class="vc_log">major cleanup</pre>
</div>



<div>
<hr />

<a name="rev6719"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6719"><strong>6719</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6719&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6719">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6719&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6719">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6719&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Fri Jan  8 11:04:43 2010 UTC</em>
(16 months, 3 weeks ago)
by <em>schloegl</em>









<br />File length: 35911 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6715&amp;r2=6719">previous 6715</a>







<pre class="vc_log">train_sc: fix nested function for matlab; test_fss: improve report</pre>
</div>



<div>
<hr />

<a name="rev6715"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6715"><strong>6715</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6715&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6715">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6715&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6715">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6715&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Thu Jan  7 20:52:33 2010 UTC</em>
(16 months, 4 weeks ago)
by <em>schloegl</em>









<br />File length: 35909 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6710&amp;r2=6715">previous 6710</a>







<pre class="vc_log">add Adaline/LMS; add ###/DELETION modes - this supports missing values also in otherwise unsupported classifiers like PLA, SVM, LMS, sparse, GSVD</pre>
</div>



<div>
<hr />

<a name="rev6710"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6710"><strong>6710</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6710&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6710">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6710&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6710">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6710&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Thu Jan  7 13:12:09 2010 UTC</em>
(16 months, 4 weeks ago)
by <em>schloegl</em>









<br />File length: 32340 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6705&amp;r2=6710">previous 6705</a>







<pre class="vc_log">a fix for PLA; hyperparameters/hyperparameter all changed to hyperparameter </pre>
</div>



<div>
<hr />

<a name="rev6705"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6705"><strong>6705</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6705&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6705">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6705&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6705">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6705&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Wed Jan  6 02:09:20 2010 UTC</em>
(16 months, 4 weeks ago)
by <em>schloegl</em>









<br />File length: 32107 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6172&amp;r2=6705">previous 6172</a>







<pre class="vc_log">fix several minor issues</pre>
</div>



<div>
<hr />

<a name="rev6172"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6172"><strong>6172</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6172&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6172">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6172&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6172">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6172&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Sat Aug 29 23:17:25 2009 UTC</em>
(21 months ago)
by <em>schloegl</em>









<br />File length: 31946 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6078&amp;r2=6172">previous 6078</a>







<pre class="vc_log">PSVM: support weighting samples</pre>
</div>



<div>
<hr />

<a name="rev6078"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6078"><strong>6078</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6078&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6078">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6078&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6078">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6078&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Wed Aug  5 19:47:24 2009 UTC</em>
(22 months ago)
by <em>schloegl</em>









<br />File length: 31587 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6074&amp;r2=6078">previous 6074</a>







<pre class="vc_log">add PLA and Winnow algorithm</pre>
</div>



<div>
<hr />

<a name="rev6074"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6074"><strong>6074</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6074&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6074">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6074&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6074">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6074&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Tue Aug  4 09:44:49 2009 UTC</em>
(22 months ago)
by <em>schloegl</em>









<br />File length: 29759 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6067&amp;r2=6074">previous 6067</a>







<pre class="vc_log">bug fix</pre>
</div>



<div>
<hr />

<a name="rev6067"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=6067"><strong>6067</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6067&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6067">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=6067&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=6067">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=6067&amp;view=log">[select for diffs]</a>




<br />

Modified

<em>Thu Jul 30 18:12:57 2009 UTC</em>
(22 months ago)
by <em>schloegl</em>









<br />File length: 29759 byte(s)







<br />Diff to <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=5985&amp;r2=6067">previous 5985</a>







<pre class="vc_log">add liblinear classifier</pre>
</div>



<div>
<hr />

<a name="rev5985"></a>


Revision <a href="/viewvc/octave?view=revision&amp;revision=5985"><strong>5985</strong></a> -


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=5985&amp;view=markup">view</a>)


(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=5985">download</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?revision=5985&amp;content-type=text%2Fplain">as text</a>)
(<a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?annotate=5985">annotate</a>)



- <a href="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m?r1=5985&amp;view=log">[select for diffs]</a>




<br />

Added

<em>Thu Jul  2 12:27:35 2009 UTC</em>
(23 months ago)
by <em>schloegl</em>







<br />File length: 27249 byte(s)











<pre class="vc_log">add classification methods, change license to GPL v3 or later, ver2.0</pre>
</div>

 



 <div class="sfBox">
<hr class="clear"/>
<a name="diff"></a>
This form allows you to request diffs between any two revisions of this file.
For each of the two "sides" of the diff,

enter a numeric revision.

<form method="get" action="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m" id="diff_select">
<table cellpadding="2" cellspacing="0" class="auto">
<tr>
<td>&nbsp;</td>
<td>

<input type="hidden" name="view" value="diff"/>

Diffs between

<input type="text" size="12" name="r1"
value="8314" />

and

<input type="text" size="12" name="r2" value="5985" />

</td>
</tr>
<tr>
<td>&nbsp;</td>
<td>
Type of Diff should be a
<select name="diff_format" onchange="submit()">
<option value="h" selected="selected">Colored Diff</option>
<option value="l" >Long Colored Diff</option>
<option value="f" >Full Colored Diff</option>
<option value="u" >Unidiff</option>
<option value="c" >Context Diff</option>
<option value="s" >Side by Side</option>
</select>
<input type="submit" value=" Get Diffs " />
</td>
</tr>
</table>
</form>
</div>


<form method="get" action="/viewvc/octave/trunk/octave-forge/extra/NaN/inst/train_sc.m">
<div>
<hr />
<a name="logsort"></a>
<input type="hidden" name="view" value="log"/>
Sort log by:
<select name="logsort" onchange="submit()">
<option value="cvs" >Not sorted</option>
<option value="date" >Commit date</option>
<option value="rev" >Revision</option>
</select>
<input type="submit" value=" Sort " />
</div>
</form>


<br />
</div>
</div>
</div>
<div id="ft">
<div class="yui-b">
<div class="yui-gb">
<div class="yui-u first" style="text-align: center;"> <a href="http://p.sf.net/sourceforge/support">SourceForge Help<a/> </div>
<div class="yui-u" style="text-align: center;"> <strong><a href="/viewvc-static/help_log.html">ViewVC Help</a></strong> </div>
<div class="yui-u" style="text-align: center;"> <a href="http://viewvc.tigris.org/">Powered by ViewVC 1.1.6</a> </div>
</div>
<p class="copyright">Copyright &copy; 2010 <a title="Network which provides and promotes Open Source software downloads, development, discussion and news." href="http://geek.net">Geeknet, Inc.</a> All rights reserved. <a href="http://p.sf.net/sourceforge/terms">Terms of Use</a></p>
</div>
</div>
</div>
<!--[if IE]></div><![endif]-->
<!--[if IE 6]></div><![endif]-->
<!--[if IE 7]></div><![endif]-->
<script type="text/javascript">
var pageTracker = _gat._getTracker("UA-32013-37");
pageTracker._setDomainName(".sourceforge.net");
pageTracker._trackPageview();
</script>
</body>
</html>


