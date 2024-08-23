import sys
import requests
from colorama import init, Fore

init(autoreset=True)


def fetch_github_activity(username):
    url = f"https://api.github.com/users/{username}/events"
    headers = {'User-Agent': 'Python CLI'}

    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(Fore.RED + f"Error fetching data: {e}")
        sys.exit(1)

    return response.json()


def parse_activity(events):
    activities = []
    for event in events:
        event_type = event.get("type")
        repo_name = event.get("repo", {}).get("name")
        message = ""

        if event_type == 'PushEvent':
            # Safely extract the commit message
            #
            commits = event.get("payload", {}).get("commits", [])
            if commits:
                message = commits[0].get("message", "")

        if event_type and repo_name:
            activities.append((event_type, repo_name, message))

    return activities


def display_activities(activities):
    if not activities:
        print("No recent activity found.")
    else:
        for event_type, repo_name, message in activities:
            if event_type == 'PushEvent':
                print(Fore.RED + f"{event_type}", "in", Fore.BLUE + f"{repo_name} - ", Fore.GREEN + f"{message}")
            else:
                print(Fore.RED + f"{event_type}", "in", Fore.BLUE + f"{repo_name} - ")


def main():
    if len(sys.argv) != 2:
        print(Fore.RED + "Usage: python github_activity.py <github_username>")
        sys.exit(1)

    username = sys.argv[1]
    events = fetch_github_activity(username)
    activities = parse_activity(events)
    display_activities(activities)


if __name__ == "__main__":
    main()
