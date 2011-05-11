$(function() {
  $('.lifedesk-greenbar-search-form-text')
    .blur(function() {
	  var orig = $('#lifedesk-greenbar-search-form').attr('action');
      $('#lifedesk-greenbar-search-form').attr('action', orig + $(this).val());
    })
    .keypress(function(event) {
      if (event.which == 13) {
        $(this).blur();
      }
  });
  $("#lifedesk-greenbar-search-form-dropdown").hover(
	function() {
      $("#lifedesk-greenbar-search-form-dropdown").css({"background-color":"#CCC", "cursor":"pointer", "border-color": "#555 #ccc #FFF #555"});
    },
    function() {
      $("#lifedesk-greenbar-search-form-dropdown").css({"background-color":"#f2f2f2", "cursor":"normal", "border-color": "#cccccc #555555 #555555 #cccccc"}); 
    })
  .toggle(
	function() {
	  $("fieldset#lifedesk-greenbar-search-menu").show();
    },
    function() {
	  $("fieldset#lifedesk-greenbar-search-menu").hide();
    }
  );
  $("fieldset#lifedesk-greenbar-search-menu").mouseup(function() {
    return false;
  });
  $('.lifedesk-greenbar-search-form-submit').click(function() {
    if($('#lifedesk-greenbar-search-type').is(':checked')) {
      window.location.href = 'http://www.lifedesks.org/search/?q=' + $('.lifedesk-greenbar-search-form-text').val();
      return false;	
    }
  });
  $(document).mouseup(function(e) {
    $("fieldset#lifedesk-greenbar-search-menu").hide();
  });
});