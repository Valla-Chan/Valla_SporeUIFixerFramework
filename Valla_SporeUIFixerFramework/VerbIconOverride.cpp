#include "stdafx.h"
#include "VerbIconOverride.h"
#include <Spore\CommonIDs.h>
#include <Spore\UTFWin\UILayout.h>
#include <Spore\UTFWin\ImageDrawable.h>

// This file is based off of one created by Auntie Owl for the SporeModAPI SDK

eastl::vector<ResourceKey> moddedVerbIconKeyCache = {};

void ReadModdedPNGs()
{
	eastl::vector<ResourceKey> verbIconKeys;

	// pull PNGs from VerbIcons folder
	ResourceManager.GetRecordKeyList(verbIconKeys,
		&Resource::StandardFileFilter(
			ResourceKey::kWildcardID,
			id("VerbIcons"),
			TypeIDs::png,
			ResourceKey::kWildcardID
		)
	);

	for (const ResourceKey& verbIconKey : verbIconKeys) {
		moddedVerbIconKeyCache.push_back(verbIconKey);
	}
}

void InjectIconWindows(UTFWin::IWindow* window)
{
	if (moddedVerbIconKeyCache.empty()) { // Read only once
		ReadModdedPNGs();
	}

	for (const ResourceKey& verbIconKey : moddedVerbIconKeyCache) {
		UTFWin::IWindow* iconWindow = UTFWin::IImageDrawable::AddImageWindow(
			{ verbIconKey.instanceID, TypeIDs::png, verbIconKey.groupID },
			0, 0, window);

		iconWindow->SetControlID(verbIconKey.instanceID);
		iconWindow->SetShadeColor(Color(0xFFFFFFFF));
		iconWindow->SetFillColor(Color(0x7f7f7f));
		iconWindow->SetArea({ 0, 0, 44, 44 }); // TODO: how to make this dynamic?
		iconWindow->SetFlag(UTFWin::kWinFlagVisible, true);
	}
}

member_detour(LoadIconSpui_detour, UTFWin::UILayout, bool(const ResourceKey&, bool, uint32_t)) {
	bool detoured(const ResourceKey& resourceKey, bool unkBool, uint32_t params) {
		// hacky fix for the subcategory issue
		if (resourceKey.instanceID == id("editorPartsPaletteCategoryWithSubcategories")) {
			return original_function(this, ResourceKey(id("editorPartsPaletteCategory_WithSubcategories"), resourceKey.typeID, resourceKey.groupID), unkBool, params);
		}

		bool result = original_function(this, resourceKey, unkBool, params);
		if (resourceKey.instanceID == id("VerbIcons")) { // SPUI holding all verb icons.
			InjectIconWindows(this->FindWindowByID(0xB879E6E8)); // First icon from spui. Should be the fastest to access. 
		}

		return result;
	}
};

long VerbIconOverride::AttachDetour() {
	long result = 0;
	result |= LoadIconSpui_detour::attach(GetAddress(UTFWin::UILayout, Load));
	return result;
}
