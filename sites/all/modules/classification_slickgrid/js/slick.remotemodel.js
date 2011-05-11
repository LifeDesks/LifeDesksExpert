/***
 * A simple observer pattern implementation.
 */
function EventHelper() {
	this.handlers = [];

	this.subscribe = function(fn) {
        this.handlers.push(fn);
    };

	this.notify = function(args) {
        for (var i = 0; i < this.handlers.length; i++) {
            this.handlers[i].call(this, args);
        }
    };

	return this;
}


(function() {

	function RemoteModel() {
		// private
		var PAGESIZE = 50;
		var data = {length:0};
		var searchstr = "";
		var checkedflags = null;
		var sortcol = null;
		var sortdir = 1;
		var h_request = null;
		var req = null; // ajax request
		var req_page;

		// events
		var onDataLoading = new EventHelper();
		var onDataLoaded = new EventHelper();


		function init() {
		}

		function isDataLoaded(from,to) {
			for (var i=from; i<=to; i++) {
				if (data[i] == undefined || data[i] == null)
					return false;
			}

			return true;
		}


		function clear() {
			for (var key in data) {
				delete data[key];
			}
			data.length = 0;
		}


		function ensureData(from,to) {
			if (req) {
				req.abort();
				for (var i=req.fromPage; i<=req.toPage; i++)
					data[i*PAGESIZE] = undefined;
			}

			if (from < 0)
				from = 0;

			var fromPage = Math.floor(from / PAGESIZE);
			var toPage = Math.floor(to / PAGESIZE);

			while (data[fromPage * PAGESIZE] !== undefined && fromPage < toPage)
				fromPage++;

			while (data[toPage * PAGESIZE] !== undefined && fromPage < toPage)
				toPage--;

			if (fromPage > toPage || ((fromPage == toPage) && data[fromPage*PAGESIZE] !== undefined)) {
				return;
			}

            var searchstr = $('#edit-search').val();

			var url = Drupal.settings.basePath + "classification_slickgrid/" + Drupal.settings.classification_slickgrid_vid + "/" + from + "/" + (to-from+1) + "/?search=" + searchstr;

            flags = [];
            $('.flag-filters').each(function() {
	          if($(this).is(':checked')) {
		        flags.push($(this).val());
	          }
            });

            if(flags.length > 0) {
	          flags.join(',');
	          url += '&flags=' + flags;
            }

            if(sortcol) {
	          url += "&sort=" + sortcol + "&sortdir=" + sortdir;
            }

			
			if (h_request != null) 
				window.clearTimeout(h_request);

			h_request = setTimeout(function() {
				for (var i=fromPage; i<=toPage; i++)
					data[i*PAGESIZE] = null; // null indicates a 'requested but not available yet'

				onDataLoading.notify({from:from, to:to});

				req = $.ajax({
					type: "GET",
					url: url,
					cache: true,
					dataType: "jsonp",
					success: onSuccess,
					error: function(){
						onError(fromPage, toPage);
					}
				});
				req.fromPage = fromPage;
				req.toPage = toPage;
			}, 50);
		}


		function onError(fromPage,toPage) {
			alert("error loading pages " + fromPage + " to " + toPage);
		}

		function onSuccess(resp) {
			var from = parseInt(resp.offset);
			var to = from + parseInt(resp.count);

			data.length = parseInt(resp.total);

			for (var i = 0; i < resp.names.length; i++) {
				data[from + i] = resp.names[i];
				data[from + i].index = from + i;
			}
	
	  	   onDataLoaded.notify({from:from, to:to});		
		}


		function reloadData(from,to) {
			for (var i=from; i<=to; i++)
				delete data[i];

			ensureData(from,to);
		}


		function setSort(column,dir) {
			sortcol = column;
			sortdir = dir;
			clear();
		}

		function setSearch(str) {
			searchstr = str;
			clear();
		}


		init();

		return {
			// properties
			"data": data,

			// methods
			"clear": clear,
			"isDataLoaded": isDataLoaded,
			"ensureData": ensureData,
			"reloadData": reloadData,
			"setSort": setSort,
			"setSearch": setSearch,

			// events
			"onDataLoading": onDataLoading,
			"onDataLoaded": onDataLoaded
		};
	}

	// Slick.Data.RemoteModel
	$.extend(true, window, { Slick: { Data: { RemoteModel: RemoteModel }}});
})();
