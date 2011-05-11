<?php
// $Id: user-profile.tpl.php,v 1.1 2010/04/13 18:06:38 dhorthouse Exp $

/**
 * @file user-profile.tpl.php
 * Theme the user profile page.
 *
 */
?>
<?php drupal_set_title($account->givenname . ' ' . $account->surname); ?>
<div class="profile">
  <?php print $user_profile; ?>
</div>