import sys
import requests


def fetch_github_activity(username):
    url = f"https://api.github.com/users/{username}/events"
    headers = {'User-Agent': 'Python CLI'}

    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"Error fetching data: {e}")
        sys.exit(1)

    return response.json()


def parse_activity(events):
    activities = []
    for event in events:
        event_type = event.get("type")
        repo_name = event.get("repo", {}).get("name")

        if event_type and repo_name:
            activity = f"{event_type} in {repo_name}"
            activities.append(activity)

    return activities


def display_activities(activities):
    if not activities:
        print("No recent activity found.")
    else:
        for activity in activities:
            print(f"- {activity}")


def main():
    if len(sys.argv) != 2:
        print("Usage: python github_activity.py <github_username>")
        sys.exit(1)

    username = sys.argv[1]
    events = fetch_github_activity(username)
    activities = parse_activity(events)
    display_activities(activities)


if __name__ == "__main__":
    main()
