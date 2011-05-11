if (Drupal.jsEnabled) {
  $(function() {
    var eol_loc = window.location.href;
    eol_loc = eol_loc.replace("/","");
    var eol_trloc = eol_loc.substring(eol_loc.length - 5);
    if(eol_trloc.indexOf("admin")==0 && eol_loc.indexOf("admin")>0) {
    	$(".error").hide();
    	$(".warning").hide();
    }
  });
}