#pragma once

/**
 * @file domain.h
 * Domain entities for the transport catalogue.
 *
 * This file is intended for classes/structs that are part of the application's
 * domain model and are independent of the transport catalogue itself.
 * Examples: bus routes and stops.
 *
 * Keeping domain types separate from transport_catalogue.h allows the map
 * renderer to depend only on domain entities, not on the entire catalogue.
 *
 * If the application structure does not require this separation, leave this file empty.
 */