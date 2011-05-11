$(function() {
  $.scrollTo( $('label:contains(' + Drupal.settings.taxon_description_chapter + ')'),2000 );

  window.onbeforeunload = function() { 
	  return false; 
  };

  $(".form-submit").click(function() { 
	  window.onbeforeunload = null; 
  });

});