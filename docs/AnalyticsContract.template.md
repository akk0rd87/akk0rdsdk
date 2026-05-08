# Analytics Contract Template

## 1. Scope

- Product area:
- Platforms:
- Owners:
- Last update:

## 2. Global Rules

- Use domain-specific event_name values instead of one shared event.
- Use source for subsystem/provider identity.
- Use snake_case for all values.
- Keep source and action dictionaries closed and review all additions.
- Avoid high-cardinality custom definitions unless strictly required.

## 3. Event Schema

| Field | Type | Required | Example | Notes |
|---|---|---|---|---|
| event_name | string | yes | interstitial_show | Domain event name |
| source | string | yes | ads_admob | Provider/subsystem |
| action | string | no | n/a | Not used in current schema |
| result | string | no | success | success/fail |
| ad_unit_id | string | no | ca-app-pub-xxx/yyy | Usually BigQuery only |
| achievement_id | string | no | CgkI... | Usually BigQuery only |
| error_code | string | no | no_fill | Optional failure context |

## 4. Allowed Values

### 4.1 source

- ads_admob
- ads_vk
- ads_yandex
- play_services_google

### 4.2 result

- success
- fail

### 4.3 event_name

- interstitial_show
- rewarded_video_show
- show_leaderboards
- show_achievements
- unlock_achievement

## 5. Mapping by Feature

| Feature | event_name | source | action | Required extra params |
|---|---|---|---|---|
| Interstitial ad show | interstitial_show | ads_admob/ads_vk/ads_yandex | n/a | none |
| Rewarded video show | rewarded_video_show | ads_admob/ads_vk/ads_yandex | n/a | none |
| Open leaderboards | show_leaderboards | play_services_google | n/a | none |
| Open achievements | show_achievements | play_services_google | n/a | none |
| Unlock achievement | unlock_achievement | play_services_google | n/a | achievement_id (optional) |

## 6. Firebase Custom Definitions

Recommended to register:

- source
- result (if used in reports)

Optional, usually do not register due to cardinality:

- ad_unit_id
- achievement_id
- error_code

## 7. Implementation Checklist

- [ ] Event emitted at method entry point.
- [ ] source and event_name values are from approved dictionary.
- [ ] result added where operation can fail.
- [ ] No new custom definition added without review.
- [ ] BigQuery export validated for optional high-cardinality params.

## 8. Change Log

| Date | Author | Change |
|---|---|---|
| YYYY-MM-DD | name | initial draft |
