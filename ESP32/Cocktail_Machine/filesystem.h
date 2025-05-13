#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Arduino.h>
#include "cocktail_data.h"
#include <map>

// Setup
// Initializes the filesystem and ensures necessary files are created.
// Purpose: To set up the filesystem and create required files if they don't exist.
void setup_data();

// Cocktails
/**
 * Saves the cocktail list to the filesystem.
 * 
 * @param cocktails Array of Cocktail objects to save.
 * @param count Number of cocktails in the array.
 * @return true if save is successful, false if an error occurs.
 */
bool save_cocktails(const Cocktail cocktails[], size_t count);

/**
 * Loads the cocktail list from the filesystem.
 * 
 * @param cocktails Array of Cocktail objects to load data into.
 * @param count A reference to a size_t that will hold the number of loaded cocktails.
 * @return true if load is successful, false if an error occurs.
 */
bool load_cocktails(Cocktail cocktails[], size_t& count);

// Ingredients
/**
 * Saves the ingredient list to the filesystem.
 * 
 * @param ingredients Array of Ingredient objects to save.
 * @return true if save is successful, false if an error occurs.
 */
bool save_ingredients(const Ingredient ingredients[INGREDIENT_COUNT]);

/**
 * Loads the ingredient list from the filesystem.
 * 
 * @param ingredients Array of Ingredient objects to load data into.
 * @return true if load is successful, false if an error occurs.
 */
bool load_ingredients(Ingredient ingredients[INGREDIENT_COUNT]);

// Stats
/**
 * Saves the stats to the filesystem.
 * 
 * @param stats The Stats object to save.
 * @return true if save is successful, false if an error occurs.
 */
bool save_stats(const Stats& stats);

/**
 * Loads the stats from the filesystem.
 * 
 * @param stats A reference to a Stats object to load data into.
 * @return true if load is successful, false if an error occurs.
 */
bool load_stats(Stats& stats);


#endif