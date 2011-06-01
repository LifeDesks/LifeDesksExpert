ABOUT

The core filters system allows an admin to restrict the type of input format available to a user
by user role.  It does not, however, allow the admin to restrict the available input formats
by node type.  This module fixes that.  It provides a simple checkbox-based interface where
an administrator can restrict what input filters are available to what nodes.  These rules 
are applied after the role-based restrictions, so it can never offer more input formats than the
default role-based input formats would.

Note that at this time, this module applies only to the body field of nodes that are created
by the node module or else use the exact same form structure as the node module.

REQUIREMENTS

- Drupal 6

INSTALLATION

- Copy the filterbynodetype directory to your modules directory.
- Go to admin/build/modules and enable it.
- Go to the settings form for each node type and set which input formats should(n't) be available.

AUTHOR AND CREDIT

Larry Garfield
larry at garfieldtech dot com
http://www.garfieldtech.com/

This module was originally developed for www.Star-Fleet.com.
