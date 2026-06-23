-- ─── DEVICES ────────────────────────────────────────────────────────────────
create table devices (
  id           uuid primary key default gen_random_uuid(),
  tag_id       text unique not null,
  role         text not null default 'tag',        -- 'tag' | 'receiver'
  lifecycle    text not null default 'provisioned', -- see constraint below
  label        text,
  last_seen    timestamptz,
  created_at   timestamptz default now()
);

alter table devices
  add constraint lifecycle_values
  check (lifecycle in ('provisioned','deployed','active','used','retired'));

-- ─── EVENTS ─────────────────────────────────────────────────────────────────
create table events (
  id            uuid primary key default gen_random_uuid(),
  device_id     uuid references devices(id),
  tag_id        text not null,
  seq           integer,
  type          text,
  torn          smallint,
  level         smallint,
  tag_status    text,
  triage_status text,
  disposition   text,
  battery_pct   smallint,
  rssi          smallint,
  gps_lat       double precision,
  gps_lon       double precision,
  gps_accuracy_m real,
  gps_fix       boolean,
  nfc_uid       text,
  nfc_payload   text,
  received_at   timestamptz,            -- clock from payload
  inserted_at   timestamptz default now()  -- db clock
);

create unique index on events (tag_id, seq);   -- dedup key
create index on events (tag_id, received_at desc);
create index on events (received_at desc);
create index on events (triage_status);

-- ─── TRIGGER: auto-provision device + keep last_seen current ─────────────────
create or replace function upsert_device_on_event()
returns trigger language plpgsql as $$
begin
  insert into devices (tag_id, role, lifecycle, last_seen)
  values (NEW.tag_id, 'tag', 'provisioned', NEW.inserted_at)
  on conflict (tag_id) do update
    set last_seen = excluded.last_seen,
        -- bump lifecycle: deployed → active when tag says so
        lifecycle = case
          when devices.lifecycle = 'deployed' and NEW.tag_status = 'active'
          then 'active'
          else devices.lifecycle
        end;

  -- backfill device_id on the row just inserted
  update events
    set device_id = (select id from devices where tag_id = NEW.tag_id)
  where id = NEW.id;

  return NEW;
end;
$$;

create trigger trg_upsert_device
  after insert on events
  for each row execute function upsert_device_on_event();
