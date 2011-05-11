
function deleteChapter(tid) {
	var chapter_element = $('#edit-'+tid+'-name');
	var chapter_name = chapter_element.val();
	if(chapter_name == '') {
		var chapter_name = chapter_element.text();
	}
	
	if(confirm(Drupal.t("Are you sure you want to delete ") + chapter_name + "?")) {
	  $.post("chapters/delete_chapter/", {"tid" : tid }, function(data) {displayMessage(data);}, "json");
	  chapter_element.parent().parent().parent().fadeOut("slow");
  }
  else {
  	return false;
  }
}

function displayMessage(content) {
    	 var message_type = "";
    	 switch (content.status) {
    	 	  case "deleted":
    	 	    message_type = "status";
    	 	    break;
    	 	  case "failed":
    	 	    message_type = "error";
    	 	    break;
    	 	  default:
    	 	    message_type = "";
    	 }
	$('#taxon_description_messages').fadeIn("fast").html('<div class="messages ' + message_type + '">' + content.message + '</div>').fadeOut(4000);
	
}