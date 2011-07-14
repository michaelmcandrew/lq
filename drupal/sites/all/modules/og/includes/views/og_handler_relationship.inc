<?php
// $Id: og_handler_relationship.inc,v 1.1.2.1 2010/12/15 09:14:11 amitaibu Exp $

/**
 * @file
 * Contains various relationship handlers.
 */

/**
 * Specialized relationship handler associating flags and content.
 *
 * @ingroup views
 */
class og_handler_relationship extends views_handler_relationship {
  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    $entity = $this->definition['entity'];

    $this->definition['extra'][] = array(
      'field' => 'entity_type',
      'value' => $this->definition['entity'],
    );
    parent::query();
  }
}