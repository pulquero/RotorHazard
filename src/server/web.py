import functools
import requests
import re as regex

@functools.lru_cache(maxsize=128)
def get_pilot_data(url):
    web_data = {'url': url}
    if url.startswith('https://league.ifpv.co.uk/pilots/'):
        resp = requests.get(url)
        name_match = regex.search("<div class=\"row vertical-center\">\s+<div class=\"col-md-3\">\s+<h1>(.*)(?=<)</h1>\s+<p>(.*)(?=<)</p>", resp.text)
        if name_match:
            web_data['callsign'] = name_match.group(1)
            web_data['name'] = name_match.group(2)
        logo_match = regex.search('https://league.ifpv.co.uk/storage/images/pilots/[0-9]+\.(jpg|png|gif)', resp.text)
        if logo_match:
            web_data['logo'] = logo_match.group(0)
    elif url.startswith('https://www.multigp.com/pilots/view/?pilot='):
        # bypass CORS
        mgp_id = regex.search("\?pilot=(.*)", url).group(1)
        web_data['callsign'] = mgp_id
        profile_url = 'https://www.multigp.com/mgp/user/view/'+mgp_id
        headers = {'Referer': url, 'Host': 'www.multigp.com', 'X-Requested-With': 'XMLHttpRequest'}
        resp = requests.get(profile_url, headers=headers)
        logo_match = regex.search("<img id=\"profileImage\"(?:.*)(?=src)src=\"([^\"]*)\"", resp.text)
        if logo_match:
            web_data['logo'] = logo_match.group(1)
    else:
        resp = requests.head(url)
        if resp.headers['Content-Type'].startswith('image/'):
            web_data['logo'] = url
    return web_data