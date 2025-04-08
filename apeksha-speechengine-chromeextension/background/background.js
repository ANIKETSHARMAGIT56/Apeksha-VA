chrome.tabs.onUpdated.addListener(async (tabId, info, tab) => {
    if (!tab.url) return;
    const url = new URL(tab.url);
    // Enables the side panel on google.com
    if (url.origin != "chrome-extension") {
        // Disables the side panel on all other sites
        await chrome.sidePanel.setOptions({
          tabId,
          enabled: false
        });

    } else {
        await chrome.sidePanel.setOptions({
            tabId,
            path: 'sidepanel.html',
            enabled: true
          });
    }
});
chrome.tabs.create({
    url: "pinnedtab.html",
    pinned: true
  }, function(tab) {
    console.log('created', tab);
});