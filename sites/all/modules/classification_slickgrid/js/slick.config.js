$(function(){
	var grid;
	var columnpicker;
	var data = [];
	var loader = new Slick.Data.RemoteModel();

	var columns = Drupal.settings.classification_slickgrid_columns;

	var options = {
		rowHeight: 24,
		editable: true,
		enableAddRow: false,
		enableCellNavigation: true
	};

	var loadingIndicator = null;
	
	grid = new Slick.Grid($("#myGrid"), loader.data, columns, options);
	var columnpicker = new Slick.Controls.ColumnPicker(columns, grid);

	grid.onViewportChanged = function() {
        var vp = grid.getViewport();
        loader.ensureData(vp.top, vp.bottom);
    };

	grid.onSort = function(sortCol, sortAsc) {
        loader.setSort(sortCol.field, sortAsc ? 1 : -1);
        var vp = grid.getViewport();
        loader.ensureData(vp.top, vp.bottom);
    };

    grid.onBeforeEditCell = function(row,cell,dataContext) {
	  if(dataContext.taxonomicrelationships && columns[cell].name == Drupal.t("Taxonomic Ranks")) {
		return false;
	  }
	  if(dataContext.taxonomicrelationships && dataContext.taxonomicrelationships !== Drupal.t("vernacular name") && columns[cell].name == Drupal.t("Vernacular Language Codes")) {
		return false;
	  }
    };

	loader.onDataLoading.subscribe(function() {
		if (!loadingIndicator)
		{
			loadingIndicator = $("<span class='loading-indicator'><label>Buffering...</label></span>").appendTo(document.body);
			var $g = $("#myGrid");

			loadingIndicator
				.css("position", "absolute")
				.css("top", $g.position().top + $g.height()/4 - loadingIndicator.height()/2)
				.css("left", $g.position().left + $g.width()/2 - loadingIndicator.width()/2)
		}

		loadingIndicator.show();
	});

	loader.onDataLoaded.subscribe(function(args) {
		for (var i = args.from; i <= args.to; i++) {
			grid.removeRow(i);
		}

		grid.updateRowCount();
		grid.render();

		loadingIndicator.fadeOut();
	});
	
	$(".txtSearch").keyup(function(e) {
	  if (e.which == 13) {
	    loader.setSearch($(this).val());
	    return false;
	  }
    });

	// load the first page
	grid.onViewportChanged();
	
	$("ul.classification_topnav li").eq(0).click(function() {
		$(this).find("ul.classification_subnav").css('z-index','6').slideDown('fast').show();
		$("ul.classification_subnav").hover(function() { }, function(){
		  $(this).slideUp('slow');
		});
	});

});	