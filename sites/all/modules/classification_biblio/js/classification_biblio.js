function classification_biblio() {
	var tid;
	var nid;
	var changed;
	
	return {
		
	// Constructor	
	init: function() {
 		this.add_events();
    },

	// Add the events
    add_events: function() {
		$(document).ajaxComplete(function(){
			var vid = Drupal.settings.classification_primary;
			$('div#classification_tree_' + vid + ' li a').click(CITE.get_citation);
		});
		$('.classification_submit').click(CITE.update_citation);
		$('#classification_name_metadata').change(function(){
			CITE.changed = true;
		});
    },

	// Get the citation data for the selected item
    get_citation: function() {
		CITE.tid = $(this).parent('LI').attr('id').replace('n', '');
		if($('#classification_name_metadata:visible').length > 0) {
		  $.get(Drupal.settings.classification_biblio_callback_base_url+"/get_citation/" + CITE.tid, { }, function(data) { CITE.build_citation(data); }, "json");
		}
    	
    },

	make_link: function(link_obj, tid, nid){
		var ul = document.createElement('ul');
		ul.setAttribute('class','classification_biblio_list');
		var li = document.createElement('li');
		var icon = (link_obj.text.indexOf("Edit")>=0) ? "edit" : "add";
		li.setAttribute('class',icon);
		var a = document.createElement('a');
        var t = document.createTextNode(link_obj.text);
        a.appendChild(t);
        li.appendChild(a);
		ul.appendChild(li);
		
		link_obj.href = link_obj.href.replace('%nid', nid);
		link_obj.href = link_obj.href.replace('%tid', tid);
		
		a.setAttribute('href', link_obj.href);
		
		a.onclick = function(){
			if(CITE.changed){
				if(!confirm(Drupal.t('The information on this form has been changed. If you continue you will lose these changes.\rIf you want to keep them, select \'Cancel\' and save the form before proceeding'))){
					return false;
				}
			}
		}
		
		return ul;
		
	},

	// Add the citation data to the form
    build_citation: function(data) {
		$('#edit-bibliographic-citation').val(data.citation);
		var link_settings;
		if(data.nid) {
			link_settings = Drupal.settings.classification_biblio_edit_link;
		}
		else {
			link_settings = Drupal.settings.classification_biblio_add_link;
		}
		
		var link = CITE.make_link(link_settings, CITE.tid, data.nid);
		$('#citation-link').html(link);

    },

    update_citation: function() {
	    if($('#edit-bibliographic-citation').val()) {
		  $.post(Drupal.settings.classification_biblio_callback_base_url+"/update_citation/", { "tid" : CITE.tid, "citation" : $('#edit-bibliographic-citation').val()});
	    }
    }

  }

}



$(function() {
    $.ajaxSetup({
	  xhr:function() { return new XMLHttpRequest(); }
    });
	CITE = new classification_biblio();
	CITE.init();
});